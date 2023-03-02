import base64
import sys
import json
import zlib
import struct

def write_str(f, s):
    b = bytes(s, 'ascii')
    f.write(struct.pack('<I', len(b)))
    f.write(b)

def write_layer(f, layer):
    data = zlib.decompress(base64.b64decode(layer['data']))
    f.write(data)

def write_dict(f, d):
    print(d)
    for k, v in d:
        write_str(f, k)
        f.write(struct.pack('<f', v))

def write_variant(f, v):
    if v['type'] == 'float':
        write_str(f, v['name'])
        f.write(struct.pack('c', bytes([0])))
        f.write(struct.pack('<f', v['value']))
    elif v['type'] == 'string':
        write_str(f, v['name'])
        f.write(struct.pack('c', bytes([1])))
        write_str(f, v['value'])
    elif v['type'] == 'int':
        write_str(f, v['name'])
        f.write(struct.pack('c', bytes([2])))
        f.write(struct.pack('<I', v['value']))
    else:
        raise Exception('expected map property to be float, int, string, got {}'.format(v['type']))

def write_props(f, props):
    f.write(struct.pack('<I', len(props)))
    for prop in props:
        print(prop)
        write_variant(f, prop)


def write_objects(f, layer):
    f.write(struct.pack('<I', len(layer['objects'])))
    for obj in layer['objects']:
        print(obj)
        write_str(f, obj['name'])
        write_str(f, obj['class'])
        f.write(struct.pack('<II', obj['x'], obj['y']))
        write_props(f, obj.get('properties', []))

with open(sys.argv[1], 'r') as f:
    mapdata = json.loads(f.read())

with open(sys.argv[2], 'wb') as f:
    print('write width/height')
    f.write(struct.pack("<II", mapdata["width"], mapdata["height"]))

    layertbl = {layer['name']: i for i, layer in enumerate(mapdata['layers'])}

    print('write base layer')
    write_layer(f, mapdata['layers'][layertbl['base']])
    print('write detail layer')
    write_layer(f, mapdata['layers'][layertbl['detail']])
    print('write fringe layer')
    write_layer(f, mapdata['layers'][layertbl['fringe']])

    write_objects(f, mapdata['layers'][layertbl['objects']])

    write_props(f, mapdata.get('properties', {}))

    print('done')
