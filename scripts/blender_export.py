import bmesh
import bpy
import struct
import sys
from bpy_extras.io_utils import axis_conversion

if "--" not in sys.argv:
    argv = []
else:
    argv = sys.argv[sys.argv.index("--") + 1:]

if len(argv) == 0:
    sys.exit(1)
else:
    filename = argv[0]

class Bounds:
    def __init__(self):
        self.left = 0
        self.right = 0
        self.floor = 0
    def __str__(self):
        return 'Bounds(left={self.left}, right={self.right}, floor={self.floor})'.format(self=self)

def compute_bounds(obj):
    mesh = bmesh.new()
    mesh.from_mesh(obj.data)
    m = axis_conversion(
        from_forward='-Y', from_up='Z',
        to_forward='Z', to_up='Y').to_4x4()
    mesh.transform(m @ obj.matrix_world)
    b = Bounds()
    b.left = float('Inf')
    b.right = float('-Inf')
    b.floor = float('Inf')
    for vert in mesh.verts:
        b.left = min(vert.co.x, b.left)
        b.right = max(vert.co.x, b.right)
        b.floor = min(vert.co.y, b.floor)
    return b

def export_all(f):
    f.write(struct.pack("<I", 0))
    nverts = 0

    bounds = Bounds()

    for object in bpy.context.scene.objects:
        if object.type != "MESH":
            continue

        if object.name == "BOUNDS":
            bounds = compute_bounds(object)
            print('found bounds at {}'.format(bounds))
            continue

        mesh = bmesh.new()

        mesh.from_mesh(object.data)
        bmesh.ops.triangulate(mesh, faces=mesh.faces)

        m = axis_conversion(
            from_forward='-Y', from_up='Z',
            to_forward='Z', to_up='Y').to_4x4()

        mesh.transform(m @ object.matrix_world)
        mesh.normal_update()

        uv_layer = mesh.loops.layers.uv.active

        for face in mesh.faces:
            for loop in face.loops:
                v = loop.vert.co
                f.write(struct.pack("fff", v.x, v.y, v.z))

                t = loop[uv_layer].uv
                f.write(struct.pack("ff", t.x, t.y))

                n = loop.vert.normal
                #n = face.normal
                f.write(struct.pack("fff", n.x, n.y, n.z))

                nverts += 1

        mesh.free()

    f.write(struct.pack("fff", bounds.left, bounds.right, bounds.floor))

    f.seek(0)
    f.write(struct.pack("<I", nverts))
    return nverts

with open(filename, "wb") as f:
    nwrote = export_all(f)
    print("Wrote {} vertices".format(nwrote))
