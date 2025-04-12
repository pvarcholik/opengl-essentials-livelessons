#include "pch.h"
#include "Mesh.h"
#include "Model.h"

using namespace std;
using namespace glm;

namespace Library
{
    Mesh::Mesh(Model& model, aiMesh& mesh) :
		mModel(&model), mName(mesh.mName.C_Str())
    {
		mMaterial = mModel->Materials().at(mesh.mMaterialIndex);

        // Vertices
        mVertices.reserve(mesh.mNumVertices);
        for (uint32_t i = 0; i < mesh.mNumVertices; i++)
        {
			const auto& vertex = mesh.mVertices[i];
			mVertices.emplace_back(vertex.x, vertex.y, vertex.z);
        }

        // Normals
        if (mesh.HasNormals())
        {
            mNormals.reserve(mesh.mNumVertices);
            for (uint32_t i = 0; i < mesh.mNumVertices; i++)
            {
				const auto& normal = mesh.mNormals[i];
				mNormals.emplace_back(normal.x, normal.y, normal.z);
            }
        }

        // Tangents and Binormals
        if (mesh.HasTangentsAndBitangents())
        {
            mTangents.reserve(mesh.mNumVertices);
            mBiNormals.reserve(mesh.mNumVertices);
            for (uint32_t i = 0; i < mesh.mNumVertices; i++)
            {
				const auto& tangent = mesh.mTangents[i];
				mTangents.emplace_back(tangent.x, tangent.y, tangent.z);

				const auto& binormal = mesh.mBitangents[i];
				mBiNormals.emplace_back(binormal.x, binormal.y, binormal.z);
            }
        }

        // Texture Coordinates
        uint32_t uvChannelCount = mesh.GetNumUVChannels();
        for (uint32_t i = 0; i < uvChannelCount; i++)
        {
            vector<vec3> textureCoordinates;
            textureCoordinates.reserve(mesh.mNumVertices);
            aiVector3D* aiTextureCoordinates = mesh.mTextureCoords[i];
            for (uint32_t j = 0; j < mesh.mNumVertices; j++)
            {
				const auto& uv = aiTextureCoordinates[j];
                textureCoordinates.emplace_back(uv.x, uv.y, uv.z);
            }

			mTextureCoordinates.push_back(move(textureCoordinates));
        }

        // Vertex Colors
        uint32_t colorChannelCount = mesh.GetNumColorChannels();
        for (uint32_t i = 0; i < colorChannelCount; i++)
        {
            vector<vec4> vertexColors;
            vertexColors.reserve(mesh.mNumVertices);
            aiColor4D* aiVertexColors = mesh.mColors[i];
            for (uint32_t j = 0; j < mesh.mNumVertices; j++)
            {
				const auto& c = aiVertexColors[j];
				vertexColors.emplace_back(c.r, c.g, c.b, c.a);
            }
			mVertexColors.push_back(move(vertexColors));
        }

        // Faces (note: could pre-reserve if we limit primitive types)
        if (mesh.HasFaces())
        {
            mFaceCount = mesh.mNumFaces;
            for (uint32_t i = 0; i < mFaceCount; i++)
            {
                aiFace* face = &mesh.mFaces[i];
            
                for (uint32_t j = 0; j < face->mNumIndices; j++)
                {		
                    mIndices.push_back(face->mIndices[j]);
                }
            }
        }
    }

    Model& Mesh::GetModel()
    {
        return *mModel;
    }

    shared_ptr<ModelMaterial> Mesh::GetMaterial() const
    {
        return mMaterial;
    }

    const string& Mesh::Name() const
    {
        return mName;
    }

    const vector<vec3>& Mesh::Vertices() const
    {
        return mVertices;
    }

    const vector<vec3>& Mesh::Normals() const
    {
        return mNormals;
    }

    const vector<vec3>& Mesh::Tangents() const
    {
        return mTangents;
    }

    const vector<vec3>& Mesh::BiNormals() const
    {
        return mBiNormals;
    }

    const vector<vector<vec3>>& Mesh::TextureCoordinates() const
    {
        return mTextureCoordinates;
    }

    const vector<vector<vec4>>& Mesh::VertexColors() const
    {
        return mVertexColors;
    }

    uint32_t Mesh::FaceCount() const
    {
        return mFaceCount;
    }

    const vector<uint32_t>& Mesh::Indices() const
    {
        return mIndices;
    }

    void Mesh::CreateIndexBuffer(GLuint& indexBuffer)
    {
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * mIndices.size(), &mIndices[0], GL_STATIC_DRAW);
    }
}