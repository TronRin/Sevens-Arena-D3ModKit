# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# ##### END GPL LICENSE BLOCK #####

from os import path
import time
from contextlib import contextmanager
from bpy.props import BoolProperty, CollectionProperty, StringProperty
from bpy.types import Operator, PropertyGroup
from bpy_extras.io_utils import ImportHelper
import bmesh
import bpy

bl_info = {
    "name": "ASCII Scene (ASE) import",
    "author": "The Dark Mod team (chedap & OrbWeaver)",
    "version": (2021, 9, 27),
    "blender": (2, 80, 0),
    "location": "File > Import-Export",
    "description": "Import .ASE format",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export",
}


class GeomObjectASE:
    def __init__(self):
        self.file = ""
        self.name = "unnamed mesh"
        self.tm = [[1.0, 0.0, 0.0, 0.0],
                   [0.0, 1.0, 0.0, 0.0],
                   [0.0, 0.0, 1.0, 0.0],
                   [0.0, 0.0, 0.0, 1.0]]
        self.numv = 0
        self.numf = 0
        self.verts = []
        self.faces = []
        self.smooth = []
        self.mtlid = []
        self.tverts = [[]]
        self.tfaces = [[]]
        self.cverts = []
        self.cfaces = []
        self.nverts = []
        self.mat = None


@contextmanager
def BMesh(mesh, read_only: bool = False):
    """Context manager to manage a bmesh which is freed after use.

    If the read_only option is False, the bmesh will write its data to the mesh before being freed."""

    # Construct a new BMesh
    bm = bmesh.new()

    # Initialise the BMesh with data from the actual mesh
    bm.from_mesh(mesh)
    bm.verts.ensure_lookup_table()
    bm.faces.ensure_lookup_table()

    # Yield the BMesh for editing
    yield bm

    # Write to the mesh if we are not read_only
    if not read_only:
        bm.to_mesh(mesh)

    # Cleanup
    bm.free()


class ImportASE(Operator, ImportHelper):
    """Import ASCII Scene format"""
    bl_idname = "import_scene.ase"
    bl_label = "Import ASCII Scene (.ase)"
    bl_options = {'REGISTER', 'UNDO'}

    option_smoothgroups: BoolProperty(
        name="Smoothing groups",
        description="Convert smoothing groups to sharp edges",
        default=True
    )
    option_normals: BoolProperty(
        name="Split normals",
        description="Import split vertex-per-face normals",
        default=False
    )
    option_reuse_mats: BoolProperty(
        name="Reuse materials",
        description="Avoid creating duplicate materials",
        default=True
    )
    option_textures: BoolProperty(
        name="Look up textures",
        description="Try loading material bitmaps (slow)",
        default=False
    )
    filename_ext = ".ase"
    filter_glob: StringProperty(
        default="*.ase",
        options={'HIDDEN'}
    )
    directory: StringProperty(subtype='DIR_PATH')

    # Filetype filter
    filter_glob: StringProperty(default="*.ase", options={'HIDDEN'})

    # File to import
    filepath: StringProperty(
        name="File Path", description="ASE file to import", maxlen=1024, default=""
    )

    file_materials = []
    cycles = False

    def draw(self, context):
        layout = self.layout

        box = layout.box()
        box.prop(self, 'option_reuse_mats')
        box.prop(self, 'option_textures')
        box.prop(self, 'option_smoothgroups')
        box.prop(self, 'option_normals')

    def execute(self, context):
        self.cycles = (bpy.context.scene.render.engine == 'CYCLES')

        file_start = time.perf_counter()
        with open(self.filepath, 'r', encoding='utf-8') as ase_file:
            self.parse_ase_file(ase_file)
        self.file_materials = []
        self.report({'INFO'}, "ASE import: {} done, took {:.0f} ms"
                    .format(self.filepath, (time.perf_counter() - file_start) * 1000))

        return {'FINISHED'}

    def find_or_create_material(self, mat_name: str, materials_mode: bool):
        """Lookup or create a material with the given name in the Blender file"""
        mat = None
        if (self.option_reuse_mats and (mat_name in bpy.data.materials)):
            # Found existing material
            if materials_mode:
                self.file_materials.append(mat_name)
            else:
                self.file_materials[-1].append(mat_name)
        else:
            # Create a new material
            mat = bpy.data.materials.new(mat_name)
            if materials_mode:
                self.file_materials.append(mat.name)
            else:
                self.file_materials[-1].append(mat.name)

        return mat

    def parse_ase_file(self, ase_file):
        materials_mode = True  # True = Materials, False = SubMaterials
        mesh = mat = tex = None
        for line in ase_file:
            words = line.split()
            if len(words) < 1:
                continue
            elif words[0] == '*MATERIAL':
                materials_mode = True
            elif words[0] == '*SUBMATERIAL' and materials_mode:
                materials_mode = False
                self.file_materials[-1] = [self.file_materials[-1]]
            elif words[0] == '*MATERIAL_NAME':
                mat_name = ' '.join(words[1:])[1:-1]
                mat = self.find_or_create_material(mat_name, materials_mode)
            elif words[0] == '*MATERIAL_DIFFUSE' and mat:
                mat.diffuse_color = [float(i) for i in words[1:]] + [1.0]
            elif words[0] == '*MATERIAL_SPECULAR' and mat:
                mat.specular_color = [float(i) for i in words[1:]]
            elif words[0] == '*MAP_NAME' and mat:
                concat_name = ' '.join(words[1:])[1:-1]
                if (self.option_reuse_mats and (concat_name in bpy.data.textures)):
                    tex = None
                    tex = bpy.data.textures[concat_name]
                else:
                    tex = bpy.data.textures.new(concat_name, 'IMAGE')
            elif words[0] == '*BITMAP' and mat and tex:
                concat_name = ' '.join(words[1:])[1:-1]
                if self.option_textures and path.isfile(concat_name):
                    tex.image = bpy.data.images.load(concat_name, True)
                elif (self.option_reuse_mats and (concat_name in bpy.data.images)):
                    tex.image = bpy.data.images[concat_name]
                else:
                    tex.image = bpy.data.images.new(concat_name, 16, 16)
                    tex.image.generated_color = (1.0, 1.0, 1.0, 1.0)
                if self.cycles:
                    mat.use_nodes = True
                    texnode = mat.node_tree.nodes.new("ShaderNodeTexImage")
                    texnode.image = tex.image
                    mat.node_tree.links.new(texnode.outputs['Color'],
                                            mat.node_tree.nodes[1].inputs['Color'])
                mat = None
                tex = None
            elif words[0] == '*GEOMOBJECT':
                if mesh:
                    self.report({'WARNING'}, "{}: undefined material on {}"
                                .format(mesh.file, mesh.name))
                    self.validate_ase_mesh(mesh)
                mesh = GeomObjectASE()
                mesh.file = path.split(ase_file.name)[1]
            elif words[0] == '*NODE_NAME' and mesh:  # Appears twice
                if mesh.name == "unnamed mesh":
                    mesh.name = ' '.join(words[1:])[1:-1]
            elif words[0] == '*TM_ROW0' and mesh:
                mesh.tm[0] = [float(i) for i in words[1:]]
            elif words[0] == '*TM_ROW1' and mesh:
                mesh.tm[1] = [float(i) for i in words[1:]]
            elif words[0] == '*TM_ROW2' and mesh:
                mesh.tm[2] = [float(i) for i in words[1:]]
            elif words[0] == '*TM_ROW3' and mesh:
                mesh.tm[3] = [float(i) for i in words[1:]]
            elif words[0] == '*MESH_NUMVERTEX' and mesh:
                mesh.numv = int(words[1])
            elif words[0] == '*MESH_NUMFACES' and mesh:
                mesh.numf = int(words[1])
            elif words[0] == '*MESH_VERTEX' and mesh:
                mesh.verts.append([float(i) for i in words[2:]])
                mesh.verts[-1].append(1)  # For transform matrix later on
            elif words[0] == '*MESH_FACE' and mesh:
                mesh.faces.append([int(words[i]) for i in range(3, 8, 2)])
                mesh.mtlid.append(int(words[-1]))
                if '*MESH_SMOOTHING' not in words:  # Compatibility
                    mesh.smooth.append({0})
                elif words[-4] == '*MESH_SMOOTHING':
                    mesh.smooth.append({0 if i == '' else int(i)
                                        for i in words[-3].split(',')})
                elif words[-3] == '*MESH_SMOOTHING':  # Compatibility
                    mesh.smooth.append({0})
            elif words[0] == '*MESH_TVERT' and mesh:
                mesh.tverts[-1].append([float(i) for i in words[2:4]])
            elif words[0] == '*MESH_TFACE' and mesh:
                mesh.tfaces[-1].append([int(i) for i in words[2:]])
            elif words[0] == '*MESH_MAPPINGCHANNEL' and mesh:
                mesh.tverts.append([])
                mesh.tfaces.append([])
            elif words[0] == '*MESH_VERTCOL' and mesh:
                mesh.cverts.append([float(i) for i in words[2:]])
            elif words[0] == '*MESH_CFACE' and mesh:
                mesh.cfaces.append([int(i) for i in words[2:]])
            elif (words[0] == '*MESH_VERTEXNORMAL' and mesh
                  and self.option_normals):
                try:
                    mesh.nverts.append([float(i) for i in words[2:]])
                except ValueError:  # Compatibility
                    mesh.nverts.append([float(i) for i in
                                        ['NaN', 'NaN', 'NaN']])
            elif words[0] == '*MATERIAL_REF' and mesh:
                mesh.mat = int(words[1])
                self.validate_ase_mesh(mesh)
                mesh = None
            elif words[0] == '*WIREFRAME_COLOR' and mesh:
                self.report({'WARNING'}, "{}: blank material on {}"
                            .format(mesh.file, mesh.name))
                self.validate_ase_mesh(mesh)
                mesh = None
        if mesh:
            self.report({'WARNING'}, "{}: undefined material on {}"
                        .format(mesh.file, mesh.name))
            self.validate_ase_mesh(mesh)

    def import_materials(self, ase_object: GeomObjectASE, mesh: bpy.types.Mesh):
        """Import materials from ASE file and assign them to the mesh"""
        if ase_object.mat is None:
            pass
        elif ase_object.mat >= len(self.file_materials):
            self.report({'ERROR'}, "{}: bad materials, skipping on {}"
                        .format(ase_object.file, ase_object.name))
        elif isinstance(self.file_materials[ase_object.mat], str):
            mesh.materials.append(bpy.data.materials[
                self.file_materials[ase_object.mat]])
        elif isinstance(self.file_materials[ase_object.mat], list):
            for submat in self.file_materials[ase_object.mat][1:]:
                mesh.materials.append(bpy.data.materials[submat])
            for face in mesh.polygons:
                face.material_index = ase_object.mtlid[face.index]

    def import_uv_data(self, ase_object: GeomObjectASE, bm: bmesh.types.BMesh):
        """Import UV layers and coordinates and assign them to the mesh"""

        # Do nothing if there is no UV data
        if not ase_object.tverts[0]:
            return

        # For each list of UVs ("UV channel") in the tfaces list-of-lists
        for chan_idx, uv_faces in enumerate(ase_object.tfaces):

            # Only try to import if we have the correct number of faces
            num_tfaces = len(ase_object.tfaces[chan_idx])
            if num_tfaces == len(bm.faces):
                # Create the destination UV layer in the BMesh
                uv_layer = bm.loops.layers.uv.new(f"UV channel {chan_idx + 1}")

                # Each UV face is a list of tvert indices
                tverts = ase_object.tverts[chan_idx]
                for faceid, uv_face in enumerate(uv_faces):
                    # Lookup the actual UV coordinates from tvert indices
                    uv_coords = [tverts[f] for f in uv_face]

                    # Assign the UVs to the destination BMesh loops
                    mesh_face = bm.faces[faceid]
                    for loopidx, loop in enumerate(mesh_face.loops):
                        loop[uv_layer].uv = uv_coords[loopidx]
            else:
                self.report(
                    {'WARNING'},
                    f"size mismatch in UV {chan_idx} [{num_tfaces} tfaces for {len(bm.faces)} faces]"
                )

    def import_smoothgroups(self, ase_object: GeomObjectASE, mesh: bpy.types.Mesh):
        """Import and apply smoothgroup information."""

        # Sanity check
        if len(ase_object.smooth) != len(mesh.polygons):
            self.report({'WARNING'}, "{}: skipped smoothgroups in {}"
                        .format(ase_object.file, ase_object.name))
            return

        with BMesh(mesh, read_only=True) as bm:
            for ed in bm.edges:
                adj_faces = ed.link_faces
                # An edge is sharp if two faces sharing it have no SGs in common
                if len(adj_faces) == 2:
                    if ase_object.smooth[adj_faces[0].index].isdisjoint(
                            ase_object.smooth[adj_faces[1].index]):
                        mesh.edges[ed.index].use_edge_sharp = True

    def validate_ase_mesh(self, aseobject):
        if (aseobject.numv != len(aseobject.verts)
                or aseobject.numf != len(aseobject.faces)):
            self.report({'ERROR'}, "{}: bad geometry, skipping mesh {}"
                        .format(aseobject.file, aseobject.name))
        else:
            aseobject.verts = [i[:-1] for i in aseobject.verts]
            aseobject.tm = [[1.0, 0.0, 0.0, 0.0],
                            [0.0, 1.0, 0.0, 0.0],
                            [0.0, 0.0, 1.0, 0.0],
                            [0.0, 0.0, 0.0, 1.0]]
            # Create the mesh
            mesh = bpy.data.meshes.new(aseobject.name)
            mesh.from_pydata(aseobject.verts, [], aseobject.faces)
            for tri in mesh.polygons:
                tri.use_smooth = True
            mesh.use_auto_smooth = True
            mesh.auto_smooth_angle = 4

            # Convert smoothing groups to sharp edges, if the option is set
            if self.option_smoothgroups:
                self.import_smoothgroups(aseobject, mesh)

            # Apply split normals
            if self.option_normals and aseobject.nverts:
                if len(aseobject.nverts) == len(mesh.loops):
                    mesh.normals_split_custom_set(aseobject.nverts)
                else:
                    self.report({'WARNING'}, "{}: skipped vertex normals in {}"
                                .format(aseobject.file, aseobject.name))
            # Apply vertex colors
            # if aseobject.cverts:
            #     if len(aseobject.cfaces) == len(mesh.polygons):
            #         mesh.vertex_colors.new("Vertex Colors")
            #         vcol = mesh.vertex_colors["Vertex Colors"].data
            #         for f in mesh.polygons:
            #             for l_idx in f.loop_indices:
            #                 vcol[l_idx].color = aseobject.cverts[
            #                     aseobject.cfaces[f.index][l_idx-f.loop_start]]
            #     else:
            #         self.report({'WARNING'}, "{}: skipped vertex colors in {}"
            #             .format(aseobject.file, aseobject.name))

            # Add UV channels
            with BMesh(mesh) as bm:
                self.import_uv_data(aseobject, bm)

            # Apply materials
            self.import_materials(aseobject, mesh)

            # Create the object and transform it
            mesh.update()
            object = bpy.data.objects.new(aseobject.name, mesh)
            object.matrix_world = aseobject.tm
            bpy.context.view_layer.active_layer_collection.collection.objects.link(object)


def menu_func(self, context):
    self.layout.operator(ImportASE.bl_idname, text="ASCII Scene (.ase)")


def register():
    bpy.utils.register_class(ImportASE)
    bpy.types.TOPBAR_MT_file_import.append(menu_func)


def unregister():
    bpy.utils.unregister_class(ImportASE)
    bpy.types.TOPBAR_MT_file_import.remove(menu_func)
