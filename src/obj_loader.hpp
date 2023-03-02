#pragma once

#include <fstream>
#include <string>
#include <vector>

struct obj_vertex
{
    float x, y, z;
};

struct obj_texcoord
{
    float u, v;
};

struct obj_vertex_normal
{
    float x, y, z;
};

struct obj_face
{
    int v1, t1, n1;
    int v2, t2, n2;
    int v3, t3, n3;
};

struct obj_mesh
{
    std::vector<obj_vertex> vertices;
    std::vector<obj_texcoord> texcoords;
    std::vector<obj_vertex_normal> normals;
    std::vector<obj_face> faces;
};

obj_mesh load_obj_mesh(std::istream& input)
{
    obj_mesh mesh;

    while (input)
    {
        std::string type;
        input >> type;

        if (type == "v")
        {
            obj_vertex& vert = mesh.vertices.emplace_back();
            input >> vert.x;
            input >> vert.y;
            input >> vert.z;
        }
        else if (type == "vt")
        {
            obj_texcoord& vert = mesh.texcoords.emplace_back();
            input >> vert.u;
            input >> vert.v;
        }
        else if (type == "vn")
        {
            obj_vertex_normal& norm = mesh.normals.emplace_back();
            input >> norm.x;
            input >> norm.y;
            input >> norm.z;
        }
        else if (type == "f")
        {
            obj_face& face = mesh.faces.emplace_back();
            input >> face.v1;
            input.get();
            input >> face.t1;
            input.get();
            input >> face.n1;

            input >> face.v2;
            input.get();
            input >> face.t2;
            input.get();
            input >> face.n2;

            input >> face.v3;
            input.get();
            input >> face.t3;
            input.get();
            input >> face.n3;

            --face.v1;
            --face.v2;
            --face.v3;
            --face.t1;
            --face.t2;
            --face.t3;
            --face.n1;
            --face.n2;
            --face.n3;
        }
        else
        {
            // ignore
            std::getline(input, type);
        }
    }

    return mesh;
}

//
//
// function parseFaceVertex(s: string) : [number, number] {
//    const parts = s.split('/');
//    return[parseInt(parts[0]) - 1, parseInt(parts[1]) - 1];
//}
//
// export type Vertex = [number, number, number];
// export type TexCoord = [number, number];
// export type Face = [[number, number], [number, number], [number, number]];
// export type Mesh = {
//  vertices: Vertex[];
//  texCoords: TexCoord[];
//  faces: Face[];
//}
//
// export function loadObj(data: string) : Mesh{
//  const lines = data.split(/ \n / g);
//
//  const vertices : Vertex[] = [];
//  const texCoords : TexCoord[] = [];
//  const faces : Face[] = [];
//
//  for (let i = 0; i < lines.length; ++i) {
//    const line = lines[i];
//    const parts = line.split(' ');
//
//    if (parts[0] == = 'v') {
//      vertices.push([
//        parseFloat(parts[1]),
//        parseFloat(parts[2]),
//        parseFloat(parts[3])
//      ]);
//    } else if (parts[0] == = 'vt') {
//      texCoords.push([
//        parseFloat(parts[1]),
//        1.0 - parseFloat(parts[2])
//      ]);
//    } else if (parts[0] == = 'f') {
//      faces.push([
//        parseFaceVertex(parts[1]),
//        parseFaceVertex(parts[2]),
//        parseFaceVertex(parts[3])
//      ]);
//    }
//  }
//
//  return {
//    vertices, texCoords, faces
//  };
//}
//
// export function buildInterleavedMesh(mesh: Mesh) : number[]{
//  const meshVerts : number[] = [];
//  for (let i = 0; i < mesh.faces.length; ++i) {
//    const face = mesh.faces[i];
//    meshVerts.push(mesh.vertices[face[0][0]][0]);
//    meshVerts.push(mesh.vertices[face[0][0]][1]);
//    meshVerts.push(mesh.vertices[face[0][0]][2]);
//    meshVerts.push(mesh.texCoords[face[0][1]][0]);
//    meshVerts.push(mesh.texCoords[face[0][1]][1]);
//
//    meshVerts.push(mesh.vertices[face[1][0]][0]);
//    meshVerts.push(mesh.vertices[face[1][0]][1]);
//    meshVerts.push(mesh.vertices[face[1][0]][2]);
//    meshVerts.push(mesh.texCoords[face[1][1]][0]);
//    meshVerts.push(mesh.texCoords[face[1][1]][1]);
//
//    meshVerts.push(mesh.vertices[face[2][0]][0]);
//    meshVerts.push(mesh.vertices[face[2][0]][1]);
//    meshVerts.push(mesh.vertices[face[2][0]][2]);
//    meshVerts.push(mesh.texCoords[face[2][1]][0]);
//    meshVerts.push(mesh.texCoords[face[2][1]][1]);
//  }
//  return meshVerts;
//}
