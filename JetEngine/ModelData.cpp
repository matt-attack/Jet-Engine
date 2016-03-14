#include "ModelData.h"

#include "Graphics\CVertexBuffer.h"
#include "Graphics\IModel.h"
#include "Graphics\Models\ObjModel.h"
#include "Graphics\CTexture.h"

ModelData::~ModelData()
{
	delete[] VertexArray;
	delete[] NormalArray;
	delete[] TangentArray;
	delete[] TexCoordArray;
	delete[] TriangleArray;

	delete[] blendindices;
	delete[] blendweights;

#ifndef MATT_SERVER
	for (int i = 0; i < this->num_meshes; i++)
	{
		//fixme later
		delete this->meshes[i].ib;
	}
#endif

	delete[] this->frames;
	delete[] this->anims;
	delete[] this->meshes;
	delete[] this->poses;
	delete[] this->joints;
	delete[] this->names;
	delete[] this->bounds;
}

ModelData* ModelData::load_as_resource(const std::string &path, ModelData* res)
{
	ModelData* d = res;//new ModelData;
	int i = path.find(".obj");
	if (i != -1)
		LoadOBJ(d, path.c_str());
	else
		LoadIQM(d, path.c_str());

#ifndef MATT_SERVER
	if (strncmp(&path[path.length() - 4], ".iqm", 4) == 0)//um, what to do if model does not exist
		BuildModelFromIqmModel(d, d);
	else
		BuildModelFromObjModel(d, d);
#endif

	d->name = path;

	return d;
}

Animation* ModelData::GetAnimation(const char* name)
{
	for (int i = 0; i < this->num_anims; i++)
	{
		if (strcmp(name, this->anims[i].name) == 0)
			return &this->anims[i];
	}
	return 0;
}

void ModelData::Reload(ResourceManager* mgr, const std::string& filename)
{
	//lets reload the model, and client model if it exists
	ModelData* ptr = this;//(ModelData*)old;
	//first modeldata
	//ptr->reconstruct<ModelData>(filename, ptr);
	//ptr->m_resmgr = mgr;

	ptr->~ModelData();

#ifdef _DEBUG
#undef new
#undef DBG_NEW
#endif
	//and then just construct again here
	new (ptr)ModelData();
#ifdef _DEBUG   
#ifndef DBG_NEW      
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )     
#define new DBG_NEW   
#endif
#endif
	ModelData *rv = ModelData::load_as_resource(filename, ptr);
}

void ModelData::BuildModelFromObjModel(ModelData* out, ModelData* mdl)
{
	int index = 0;
	MdlVert* verts = new MdlVert[mdl->NumTriangle * 3];
	out->verts = mdl->NumTriangle * 3;
	out->anims = 0;
	out->num_anims = 0;
	out->num_meshes = 1;
	out->meshes = new Mesh;
	out->bounds = 0;

	out->meshes->ib = 0;
	out->meshes->first_triangle = out->meshes->first_vertex = 0;
	out->meshes->num_vertexes = out->verts;
	out->meshes->num_triangles = 0;
	out->meshes->name = "ObjModel";
	out->meshes->material = 0;
	throw "unimplemented!";
	for (int i = 0; i < mdl->NumTriangle; i++)
	{
		for (int v = 0; v < 3; v++)
		{
			int vi = mdl->TriangleArray2[i].Vertex[v] - 1;
			verts[index].x = mdl->VertexArray[vi].X;
			verts[index].y = mdl->VertexArray[vi].Y;
			verts[index].z = mdl->VertexArray[vi].Z;

			int vn = mdl->TriangleArray2[i].Normal[v] - 1;
			if (vn < mdl->NumNormal && vn > 0)
			{
				verts[index].nx = mdl->NormalArray[vn].X;
				verts[index].ny = mdl->NormalArray[vn].Y;
				verts[index].nz = mdl->NormalArray[vn].Z;
			}

			int ti = mdl->TriangleArray2[i].TexCoord[v] - 1;
			verts[index].u = mdl->TexCoordArray[ti].U;
			verts[index].v = mdl->TexCoordArray[ti].V;
			index++;
		}
	}
	out->joints = 0;
	out->num_frames = out->num_joints = 0;

#ifndef MATT_SERVER
	out->vb.SetVertexDeclaration(renderer->GetVertexDeclaration(5));
	out->vb.Data(verts, mdl->NumTriangle * 3 * sizeof(MdlVert), sizeof(MdlVert));
#endif

	//out->ib = 0;
	out->poses = 0;

	delete[] verts;
}

void ModelData::BuildModelFromIqmModel(ModelData* out, ModelData* mdl)
{
	//fill index buffers
	for (int i = 0; i < mdl->num_meshes; i++)
	{
		Mesh* mesh = &mdl->meshes[i];
		unsigned short* indices = new unsigned short[mesh->num_triangles * 3];
		int index = 0;
		for (int ti = mesh->first_triangle; ti < (mesh->first_triangle + mesh->num_triangles); ti++)
		{
			for (int v = 0; v < 3; v++)
			{
				indices[index++] = mdl->TriangleArray[ti].Vertex[v];
			}
		}

		//fill IBO
#ifndef MATT_SERVER
		out->meshes[i].ib = new CIndexBuffer;
		out->meshes[i].ib->Data(indices, mesh->num_triangles * 3 * sizeof(unsigned short), 0);
#endif
		delete[] indices;
	}

	//fill vertex buffer
	MdlVert* verts = new MdlVert[mdl->NumVertex];
	for (int i = 0; i < mdl->NumVertex; i++)
	{
		int vi = i;//mdl->TriangleArray[i].Vertex[v];
		verts[i].x = mdl->VertexArray[vi].X;
		verts[i].y = mdl->VertexArray[vi].Y;
		verts[i].z = mdl->VertexArray[vi].Z;
		verts[i].nx = mdl->NormalArray[vi].X;
		verts[i].ny = mdl->NormalArray[vi].Y;
		verts[i].nz = mdl->NormalArray[vi].Z;
		verts[i].u = mdl->TexCoordArray[vi].U;
		verts[i].v = mdl->TexCoordArray[vi].V;

		for (int i2 = 0; i2 < 4; i2++)
			verts[i].blendindex[i2] = mdl->blendindices[vi * 4 + i2];
		for (int i2 = 0; i2 < 4; i2++)
			verts[i].blendweight[i2] = mdl->blendweights[vi * 4 + i2];
	}

	//fill super vertex buffer
	MdlVert2* verts2 = new MdlVert2[mdl->NumVertex];
	for (int i = 0; i < mdl->NumVertex; i++)
	{
		int vi = i;//mdl->TriangleArray[i].Vertex[v];
		verts2[i].x = mdl->VertexArray[vi].X;
		verts2[i].y = mdl->VertexArray[vi].Y;
		verts2[i].z = mdl->VertexArray[vi].Z;
		verts2[i].nx = mdl->NormalArray[vi].X;
		verts2[i].ny = mdl->NormalArray[vi].Y;
		verts2[i].nz = mdl->NormalArray[vi].Z;
		verts2[i].tx = mdl->TangentArray[vi].X;
		verts2[i].ty = mdl->TangentArray[vi].Y;
		verts2[i].tz = mdl->TangentArray[vi].Z;
		verts2[i].u = mdl->TexCoordArray[vi].U;
		verts2[i].v = mdl->TexCoordArray[vi].V;

		for (int i2 = 0; i2 < 4; i2++)
			verts2[i].blendindex[i2] = mdl->blendindices[vi * 4 + i2];
		for (int i2 = 0; i2 < 4; i2++)
			verts2[i].blendweight[i2] = mdl->blendweights[vi * 4 + i2];
	}

#ifndef MATT_SERVER
	out->vbt = CVertexBuffer(VertexBufferUsage::Static);
	out->vbt.SetVertexDeclaration(renderer->GetVertexDeclaration(7));
	out->vbt.Data(verts2, mdl->NumVertex*sizeof(MdlVert2), sizeof(MdlVert2));

	out->vb = CVertexBuffer(VertexBufferUsage::Static);
	out->vb.SetVertexDeclaration(renderer->GetVertexDeclaration(5));
	out->vb.Data(verts, mdl->NumVertex*sizeof(MdlVert), sizeof(MdlVert));
#endif

	delete[] verts;
	delete[] verts2;
}

int ModelData::GetAnimationID(char* name)
{
	for (int i = 0; i < this->num_anims; i++)
	{
		if (strcmp(name, this->anims[i].name) == 0)
			return i;
	}
	return 0;
}

int ModelData::GetBone(const char* name)
{
	for (int i = 0; i < num_joints; i++)
	{
		if (strcmp(name, joints[i].name) == 0)
			return i;
	}
	return -1;
};

/* Return 1 if the character is dead space, 0 if not */
int IsDeadSpace2(char A)
{
	if (A < 33) return 1;
	else return 0;
}

/* Return 1 if the character is a newline/linefeed, 0 if not */
int IsEOL2(char A)
{
	if (A == 10) return 1;
	else return 0;
}

int StrEqual2(char *A, char *B, char *E, int count)
{
	int c;
	c = 0;
	while ((c < count) && (A != E))
	{
		if (A[c] != B[c]) return 0;
		c++;
	}
	if (A == E) return 0;
	else return 1;
}

void ModelData::LoadOBJ(ModelData* ret, const char* path)
{
	printf("in load obj\n");
#ifndef ANDROID
	FILE* f = fopen(path, "r");
	if (f)
#endif
	{
#ifndef ANDROID
		fseek(f, 0, SEEK_END);   // non-portable
		int size = ftell(f);
		fseek(f, 0, SEEK_SET);
		char* buff = new char[size];
		for (int i = 0; i < size; i++)
		{
			buff[i] = fgetc(f);
		}
		fclose(f);
#else
		AAsset* a = AAssetManager_open(mgr, path, 0);
		unsigned int size = AAsset_getLength(a);
		char* buff = new char[size];
		AAsset_read(a, buff, size);
		AAsset_close(a);
#endif

		char *p, *e;
		char b[512];
		int c;

		// current position and end location pointers
		p = buff;
		e = buff + size;

		ret->NumNormal = ret->NumTriangle = ret->NumVertex = ret->NumTexCoord = 0;

		// first pass, scan the number of vertex, normals, texcoords, and faces
		while (p != e)
		{
			// nibble off one line, ignoring leading dead space
			c = 0;
			while ((IsDeadSpace2(*p)) && (p != e)) p++;
			while ((!IsEOL2(*p)) && (p != e) && (c < 512)) { b[c++] = *p; p++; }

			// ok, b[] contains the current line
			if (StrEqual2(b, "vn", &b[c], 2)) ret->NumNormal++;
			else
				if (StrEqual2(b, "vt", &b[c], 2)) ret->NumTexCoord++;
				else
					if (StrEqual2(b, "v", &b[c], 1)) ret->NumVertex++;
					else
						if (StrEqual2(b, "f", &b[c], 1)) ret->NumTriangle++;
		}

		logf("normals: %d texcoords: %d vertices: %d triangles: %d\n", ret->NumNormal, ret->NumTexCoord, ret->NumVertex, ret->NumTriangle);

		// now allocate the arrays
		ret->VertexArray = (IqmVertex*)malloc(sizeof(IqmVertex)*ret->NumVertex);
		ret->NormalArray = (IqmNormal*)malloc(sizeof(IqmNormal)*ret->NumNormal);
		ret->TexCoordArray = (IqmTexCoord*)malloc(sizeof(IqmTexCoord)*ret->NumTexCoord);
		ret->TriangleArray = 0;
		ret->TriangleArray2 = (ObjTriangles*)malloc(sizeof(ObjTriangles)*ret->NumTriangle);

		p = buff;
		int TCi, Ni, Vi, TGi;
		TCi = Ni = Vi = TGi = 0;
		while (p != e)
		{
			// nibble off one line, ignoring leading dead space
			c = 0;
			while ((IsDeadSpace2(*p)) && (p != e)) p++;
			while ((!IsEOL2(*p)) && (p != e) && (c < 512)) { b[c++] = *p; p++; }
			b[c + 1] = 0;

			// ok, b[] contains the current line
			if (StrEqual2(b, "vn", &b[c], 2))
			{
				sscanf(b, "vn %f %f %f", &ret->NormalArray[Ni].X, &ret->NormalArray[Ni].Y, &ret->NormalArray[Ni].Z);
				Ni++;
			}
			else if (StrEqual2(b, "vt", &b[c], 2))
			{
				sscanf(b, "vt %f %f", &ret->TexCoordArray[TCi].U, &ret->TexCoordArray[TCi].V);
				TCi++;
			}
			else if (StrEqual2(b, "v", &b[c], 1))
			{
				sscanf(b, "v %f %f %f", &ret->VertexArray[Vi].X, &ret->VertexArray[Vi].Y, &ret->VertexArray[Vi].Z);
				Vi++;
			}
			else if (StrEqual2(b, "f", &b[c], 1))
			{
				int Fc = TGi;
				ret->TriangleArray2[Fc].Vertex[0] = ret->TriangleArray2[Fc].TexCoord[0] = ret->TriangleArray2[Fc].Normal[0] = 0;
				ret->TriangleArray2[Fc].Vertex[1] = ret->TriangleArray2[Fc].TexCoord[1] = ret->TriangleArray2[Fc].Normal[1] = 0;
				ret->TriangleArray2[Fc].Vertex[2] = ret->TriangleArray2[Fc].TexCoord[2] = ret->TriangleArray2[Fc].Normal[2] = 0;

				sscanf(b, "f %d/%d/%d %d/%d/%d %d/%d/%d",
					&ret->TriangleArray2[Fc].Vertex[0], &ret->TriangleArray2[Fc].TexCoord[0], &ret->TriangleArray2[Fc].Normal[0],
					&ret->TriangleArray2[Fc].Vertex[1], &ret->TriangleArray2[Fc].TexCoord[1], &ret->TriangleArray2[Fc].Normal[1],
					&ret->TriangleArray2[Fc].Vertex[2], &ret->TriangleArray2[Fc].TexCoord[2], &ret->TriangleArray2[Fc].Normal[2]);
				TGi++;
			}
			else
			{

			}
		}

		delete[] buff;
	}
	else
	{
		printf("ERROR: Missing model: %s\n", path);
	}
}

void ModelData::LoadIQM(ModelData* m, const char* path)
{
	Sleep(200);
	printf("in load iqm\n");
#ifndef ANDROID
	FILE* f = fopen(path, "rb");
	if (f)
#else
	AAsset* a = AAssetManager_open(mgr, path, 0);
	if (a)
#endif
	{
		iqmheader header;
#ifndef ANDROID
		fread(&header, sizeof(iqmheader), 1, f);
#else
		AAsset_read(a, &header, sizeof(iqmheader));
#endif
		//lets put our data here
		IqmVertex* vertArray = new IqmVertex[header.num_vertexes];
		IqmTexCoord* texcoordArray = new IqmTexCoord[header.num_vertexes];
		IqmNormal* normalArray = new IqmNormal[header.num_vertexes];
		IqmNormal* tangents = new IqmNormal[header.num_vertexes];
		iqmjoint* joints = new iqmjoint[header.num_joints];
		iqmanim* anims = new iqmanim[header.num_anims];
		iqmpose* poses = new iqmpose[header.num_poses];
		unsigned char* blendindices = new unsigned char[header.num_vertexes * 4];
		unsigned char* blendweights = new unsigned char[header.num_vertexes * 4];
		unsigned short* framesd = new unsigned short[header.num_frames*header.num_framechannels];
		iqmmesh* meshes = new iqmmesh[header.num_meshes];

#ifndef ANDROID
		//read frames
		fseek(f, header.ofs_frames, SEEK_SET);
		fread(framesd, sizeof(unsigned short), header.num_frames*header.num_framechannels, f);

		//read bounds, not used yet
		iqmbounds* bounds = new iqmbounds[header.num_frames];
		fseek(f, header.ofs_bounds, SEEK_SET);
		fread(bounds, sizeof(iqmbounds), header.num_frames, f);
		//delete[] bounds;

		//read poses
		fseek(f, header.ofs_poses, SEEK_SET);
		fread(poses, sizeof(iqmpose), header.num_poses, f);

		//read anims
		fseek(f, header.ofs_anims, SEEK_SET);
		fread(anims, sizeof(iqmanim), header.num_anims, f);

		//read joints
		fseek(f, header.ofs_joints, SEEK_SET);
		fread(joints, sizeof(iqmjoint), header.num_joints, f);

		//read in meshes
		fseek(f, header.ofs_meshes, SEEK_SET);//todo: read in meshes, this is breaking the head mesh and thereby the whole thing by not doing this
		fread(meshes, sizeof(iqmmesh), header.num_meshes, f);

		//read in names
		char* names = new char[header.num_text];
		fseek(f, header.ofs_text, SEEK_SET);
		fread(names, header.num_text, 1, f);
#else
		//read frames
		AAsset_seek(a, header.ofs_frames, SEEK_SET);//fseek(f,header.ofs_vertexarrays,SEEK_SET);
		AAsset_read(a, framesd, sizeof(unsigned short)*header.num_frames*header.num_framechannels);

		//read bounds, not used yet
		iqmbounds* bounds = new iqmbounds[header.num_frames];
		//fseek(f, header.ofs_bounds, SEEK_SET);
		//fread(bounds, sizeof(iqmbounds), header.num_frames, f);
		//delete[] bounds;

		//read poses
		AAsset_seek(a, header.ofs_poses, SEEK_SET);//fseek(f,header.ofs_vertexarrays,SEEK_SET);
		AAsset_read(a, poses, sizeof(iqmpose)*header.num_poses);

		//read anims
		AAsset_seek(a, header.ofs_anims, SEEK_SET);//fseek(f,header.ofs_vertexarrays,SEEK_SET);
		AAsset_read(a, anims, sizeof(iqmanim)*header.num_anims);

		//read joints
		AAsset_seek(a, header.ofs_joints, SEEK_SET);//fseek(f,header.ofs_vertexarrays,SEEK_SET);
		AAsset_read(a, joints, sizeof(iqmjoint)*header.num_joints);

		//read in meshes
		AAsset_seek(a, header.ofs_meshes, SEEK_SET);//fseek(f,header.ofs_vertexarrays,SEEK_SET);
		AAsset_read(a, meshes, sizeof(iqmmesh)*header.num_meshes);

		//read in names
		char* names = new char[header.num_text];
		AAsset_seek(a, header.ofs_text, SEEK_SET);//fseek(f,header.ofs_vertexarrays,SEEK_SET);
		AAsset_read(a, names, header.num_text);
#endif
		//lets read our meshes yo
		Mesh* nmeshes = new Mesh[header.num_meshes];
		for (unsigned int i = 0; i < header.num_meshes; i++)
		{
			iqmmesh* m = &meshes[i];
			char* name = &names[m->name];
			char* material = &names[m->material];

			logf("Mesh: %s with material %s\n", name, material);

			Mesh* nm = &nmeshes[i];
			nm->name = name;

			//generate material
			IMaterial* mat = new IMaterial(material);
			mat->alpha = false;
			//ok, this is eww, but fine for now
			std::string mname = material;
			if (mname.length() > 0 && mname[mname.length() - 1] != 'g')
			{
				mname = mname + ".tga";
			}
			mat->skinned = true;
			mat->shader_name = "Shaders/generic.txt";
			mat->shader_builder = true;
			mat->diffuse = mname;
			mat->Update(renderer);//load any associated textures

			nm->material = mat;

			nm->first_triangle = m->first_triangle;
			nm->num_triangles = m->num_triangles;
			nm->first_vertex = m->first_vertex;
			nm->num_vertexes = m->num_vertexes;
		}

		//lets read vertex arrays
		iqmvertexarray* vas = new iqmvertexarray[header.num_vertexarrays];
#ifndef ANDRO
		fseek(f, header.ofs_vertexarrays, SEEK_SET);
		fread(vas, sizeof(iqmvertexarray), header.num_vertexarrays, f);
#else
		AAsset_seek(a, header.ofs_vertexarrays, SEEK_SET);//fseek(f,header.ofs_vertexarrays,SEEK_SET);
		AAsset_read(a, vas, sizeof(iqmvertexarray)*header.num_vertexarrays);
#endif
		for (unsigned int i = 0; i < header.num_vertexarrays; i++)
		{
			iqmvertexarray va = vas[i];

			//read out the data
#ifndef ANDROID
			fseek(f, va.offset, SEEK_SET);
			if (va.type == IQM_POSITION)
			{
				fread(vertArray, sizeof(IqmVertex), header.num_vertexes, f);
			}
			else if (va.type == IQM_TEXCOORD)
			{
				fread(texcoordArray, sizeof(IqmTexCoord), header.num_vertexes, f);
			}
			else if (va.type == IQM_NORMAL)
			{
				fread(normalArray, sizeof(IqmNormal), header.num_vertexes, f);
			}
			else if (va.type == IQM_COLOR)
			{

			}
			else if (va.type == IQM_BLENDINDEXES)
			{
				fread(blendindices, 4, header.num_vertexes, f);
			}
			else if (va.type == IQM_BLENDWEIGHTS)
			{
				fread(blendweights, 4, header.num_vertexes, f);
			}
			else if (va.type == IQM_TANGENT)
			{
				fread(tangents, sizeof(IqmNormal), header.num_vertexes, f);
			}
#else
			if (va.type == IQM_POSITION)
			{
				AAsset_seek(a, va.offset, SEEK_SET);//fseek(f,va.offset,SEEK_SET);
				AAsset_read(a, vertArray, sizeof(IqmVertex)*header.num_vertexes);//fread(vertArray, sizeof(IqmVertex), header.num_vertexes, f);
			}
			else if (va.type == IQM_TEXCOORD)
			{
				AAsset_seek(a, va.offset, SEEK_SET);//fseek(f,va.offset,SEEK_SET);
				AAsset_read(a, texcoordArray, sizeof(IqmTexCoord)*header.num_vertexes);//fread(texcoordArray, sizeof(IqmTexCoord), header.num_vertexes, f);
			}
			else if (va.type == IQM_NORMAL)
			{
				AAsset_seek(a, va.offset, SEEK_SET);//fseek(f,va.offset,SEEK_SET);
				AAsset_read(a, normalArray, sizeof(IqmNormal)*header.num_vertexes);//fread(normalArray, sizeof(IqmNormal), header.num_vertexes, f);
			}
			else if (va.type == IQM_COLOR)
			{
				AAsset_seek(a, va.offset, SEEK_SET);//fseek(f,va.offset,SEEK_SET);
			}
			else if (va.type == IQM_BLENDINDEXES)
			{
				AAsset_seek(a, va.offset, SEEK_SET);
				AAsset_read(a, blendindices, header.num_vertexes*4);
			}
			else if (va.type == IQM_BLENDWEIGHTS)
			{
				AAsset_seek(a, va.offset, SEEK_SET);
				AAsset_read(a, blendweights, header.num_vertexes*4);
			}
#endif
		}

		//read triangles
		IqmTriangle* tr = new IqmTriangle[header.num_triangles];
#ifndef ANDROID
		fseek(f, header.ofs_triangles, SEEK_SET);
		fread(tr, sizeof(IqmTriangle), header.num_triangles, f);
#else
		AAsset_seek(a, header.ofs_triangles, SEEK_SET);//fseek(f,header.ofs_triangles,SEEK_SET);
		AAsset_read(a, tr, sizeof(IqmTriangle)*header.num_triangles);//fread(tr, sizeof(IqmTriangle), header.num_triangles, f);
#endif
		Matrix3x4* frames = new Matrix3x4[header.num_frames * header.num_poses];

		//IqmModel* m = new IqmModel;
		m->VertexArray = vertArray;
		m->TexCoordArray = texcoordArray;
		m->TriangleArray = tr;
		m->NormalArray = normalArray;
		m->TangentArray = tangents;
		m->NumTexCoord = header.num_vertexes;
		m->NumTriangle = header.num_triangles;
		m->NumVertex = header.num_vertexes;
		m->NumNormal = header.num_vertexes;
		m->blendindices = blendindices;
		m->blendweights = blendweights;
		m->num_frames = header.num_frames;
		m->num_joints = header.num_joints;
		m->num_meshes = header.num_meshes;
		m->meshes = nmeshes;
		m->bounds = bounds;
		m->names = names;
		//m->joints = joints;

		m->frames = frames;
		m->num_anims = header.num_anims;
		m->anims = new Animation[header.num_anims];
		logf("Frames: %i, Anims: ",header.num_frames);
		for (int i = 0; i < header.num_anims; i++)
		{
			Animation* a = &m->anims[i];
			iqmanim* ia = &anims[i];
			a->name = &names[ia->name];
			log(a->name);/*log(", ");*/
			a->first_frame = ia->first_frame;
			a->flags = ia->flags;
			if (a->flags == 0)
			{
				if (strncmp(a->name, "die", 3) != 0)
					a->flags = IQM_LOOP;
			}
			a->num_frames = ia->num_frames;
			a->framerate = ia->framerate;
		}
		log("\n");

		Matrix3x4* baseframe = new Matrix3x4[header.num_joints];
		Quaternion* qbaseframe = new Quaternion[header.num_joints];
		Matrix3x4* inversebaseframe = new Matrix3x4[header.num_joints];
		Quaternion* qinversebaseframe = new Quaternion[header.num_joints];
		for (int i = 0; i < (int)header.num_joints; i++)
		{
			iqmjoint &j = joints[i];
			qbaseframe[i] = Quat(j.rotate).normalize();
			baseframe[i] = Matrix3x4(Quat(j.rotate).normalize(), Vec3(j.translate), Vec3(j.scale));
			qinversebaseframe[i] = qbaseframe[i].Inverse();
			inversebaseframe[i].invert(baseframe[i]);
			if (j.parent >= 0)
			{
				/*qbaseframe[i] = qbaseframe[j.parent] * qbaseframe[i];
				baseframe[i] = baseframe[j.parent] * baseframe[i];
				qinversebaseframe[i] = qinversebaseframe[i] * qinversebaseframe[j.parent];
				inversebaseframe[i] *= inversebaseframe[j.parent];*/
				qbaseframe[i] = qbaseframe[j.parent] * qbaseframe[i];
				baseframe[i] *= baseframe[j.parent];// * baseframe[i];
				qinversebaseframe[i] = qinversebaseframe[i] * qinversebaseframe[j.parent];
				inversebaseframe[i] = inversebaseframe[j.parent] * inversebaseframe[i];
			}
		}

		//int bone_swap[75] = {0};
		m->joints = new Joint[header.num_joints];
		//log("Joints: ");
		//int num_attachments = 0;
		for (int i = 0; i < header.num_joints; i++)
		{
			Joint* a = &m->joints[i];
			iqmjoint* ia = &joints[i];
			a->name = &names[ia->name];
			//log(a->name);log(", ");
			a->parent = ia->parent;
			for (int p = 0; p < 4; p++)
				a->rotate[p] = ia->rotate[p];
			for (int p = 0; p < 3; p++)
				a->scale[p] = ia->scale[p];
			for (int p = 0; p < 3; p++)
				a->translate[p] = ia->translate[p];
			a->matrix = baseframe[i];
			a->invmatrix = inversebaseframe[i];

			//if (a->name[0] == '!')
			//	num_attachments++;
		}
		//m->num_attachments = num_attachments;
		//m->attachments = new Joint
		//for (int i = 0; i < header.num_joints; i++)
		//{

		//}
		//log("\n");

		
		unsigned short *framedata = (unsigned short*)framesd;

		m->poses = new Pose[header.num_frames*header.num_poses];
		for (int i = 0; i < (int)header.num_frames; i++)
		{
			for (int j = 0; j < (int)header.num_poses; j++)
			{
				iqmpose &p = poses[j];
				Quat rotate;
				Vec3 translate, scale;
				translate.x = p.channeloffset[0]; if (p.mask & 0x01) translate.x += *framedata++ * p.channelscale[0];
				translate.y = p.channeloffset[1]; if (p.mask & 0x02) translate.y += *framedata++ * p.channelscale[1];
				translate.z = p.channeloffset[2]; if (p.mask & 0x04) translate.z += *framedata++ * p.channelscale[2];
				rotate.x = p.channeloffset[3]; if (p.mask & 0x08) rotate.x += *framedata++ * p.channelscale[3];
				rotate.y = p.channeloffset[4]; if (p.mask & 0x10) rotate.y += *framedata++ * p.channelscale[4];
				rotate.z = p.channeloffset[5]; if (p.mask & 0x20) rotate.z += *framedata++ * p.channelscale[5];
				rotate.w = p.channeloffset[6]; if (p.mask & 0x40) rotate.w += *framedata++ * p.channelscale[6];
				scale.x = p.channeloffset[7]; if (p.mask & 0x80) scale.x += *framedata++ * p.channelscale[7];
				scale.y = p.channeloffset[8]; if (p.mask & 0x100) scale.y += *framedata++ * p.channelscale[8];
				scale.z = p.channeloffset[9]; if (p.mask & 0x200) scale.z += *framedata++ * p.channelscale[9];
				// Concatenate each pose with the inverse base pose to avoid doing this at animation time.
				// If the joint has a parent, then it needs to be pre-concatenated with its parent's base pose.
				// Thus it all negates at animation time like so: 
				//   (parentPose * parentInverseBasePose) * (parentBasePose * childPose * childInverseBasePose) =>
				//   parentPose * (parentInverseBasePose * parentBasePose) * childPose * childInverseBasePose =>
				//   parentPose * childPose * childInverseBasePose

				Matrix3x4 ma(rotate.normalize(), translate, scale);
				if (p.parent >= 0)
				{
					m->poses[i*header.num_poses + j].rotation = qbaseframe[p.parent] * (rotate.normalize()) * qinversebaseframe[j];//(((Quaternion)joints[j].rotate).Inverse()) * (rotate.normalize()) * (((Quaternion)joints[p.parent].rotate));//((Quaternion)joints[p.parent].rotate) * (rotate.normalize()) * (((Quaternion)joints[j].rotate).Inverse());
					//m->poses[i*header.num_poses + j].translation = translate + (Vec3)joints[p.parent].translate - (Vec3)joints[j].translate;//Vec3(0,0,0);
					//frames[i*header.num_poses + j] = baseframe[p.parent] * ma * inversebaseframe[j];
					frames[i*header.num_poses + j] = inversebaseframe[j] * ma * baseframe[p.parent];

					m->poses[i*header.num_poses + j].translation = frames[i*header.num_poses + j].GetTranslation();
					//m->poses[i*header.num_poses + j].rotation.FromRotationMatrix((Matrix4)frames[i*header.num_poses + j]);	
				}
				else
				{
					m->poses[i*header.num_poses + j].rotation = (rotate.normalize()) * qinversebaseframe[j];//(rotate.normalize()) * (((Quaternion)joints[j].rotate).Inverse());
					//m->poses[i*header.num_poses + j].translation = translate - (Vec3)joints[j].translate;//Vec3(0,0,0);
					//frames[i*header.num_poses + j] = ma * inversebaseframe[j];

					frames[i*header.num_poses + j] = inversebaseframe[j] * ma;// * inversebaseframe[j];
					//m->poses[i*header.num_poses + j].rotation.FromRotationMatrix((Matrix4)frames[i*header.num_poses + j]);
					m->poses[i*header.num_poses + j].translation = frames[i*header.num_poses + j].GetTranslation();
				}
			}
		}

#ifdef ANDROID
		AAsset_close(a);//fclose(f);
#else
		fclose(f);
#endif
		//get bounding boxes for bones
		for (int i = 0; i < m->num_joints; i++)
			m->joints[i].bb = AABB(Vec3(0, 0, 0), Vec3(0, 0, 0));

		for (int i = 0; i < m->NumVertex; i++)
		{
			/*int vi = i;//mdl->TriangleArray[i].Vertex[v];
			verts[i].x = mdl->VertexArray[vi].X;
			verts[i].y = mdl->VertexArray[vi].Y;
			verts[i].z = mdl->VertexArray[vi].Z;
			verts[i].nx = mdl->NormalArray[vi].X;
			verts[i].ny = mdl->NormalArray[vi].Y;
			verts[i].nz = mdl->NormalArray[vi].Z;
			verts[i].u = mdl->TexCoordArray[vi].U;
			verts[i].v = mdl->TexCoordArray[vi].V;

			for (int i2 = 0; i2 < 4; i2++)
			verts[i].blendindex[i2] = mdl->blendindices[vi*4+i2];
			for (int i2 = 0; i2 < 4; i2++)
			verts[i].blendweight[i2] = mdl->blendweights[vi*4+i2];*/

			//first, pick which index has highest weight to determine bone
			int ind = 0;
			int max = 0;
			for (int z = 0; z < 4; z++)
			{
				if (m->blendweights[i * 4 + z] > max)//verts[i].blendweight[z] > max)
				{
					max = m->blendweights[i * 4 + z];//verts[i].blendweight[z];
					ind = z;
				}
			}

			if (max == 0)//lets make sure at least one bone has weight
			{
				m->blendweights[i * 4] = 255;//verts[i].blendweight[0] = 255;
				//log("help");
			}

			int bi = m->blendindices[i * 4 + ind];//verts[i].blendindex[ind];
			Vec3 pos = Vec3(m->VertexArray[i].X, m->VertexArray[i].Y, m->VertexArray[i].Z);
			//Matrix3x4 t = mdl->joints[bi].matrix;
			//t.invert();
			//pos = t.transform(pos);
			if (m->joints[bi].bb.max == Vec3(0, 0, 0) && m->joints[bi].bb.min == Vec3(0, 0, 0))
				m->joints[bi].bb.max = m->joints[bi].bb.min = pos;
			if (pos.x > m->joints[bi].bb.max.x)
				m->joints[bi].bb.max.x = pos.x;
			if (pos.y > m->joints[bi].bb.max.y)
				m->joints[bi].bb.max.y = pos.y;
			if (pos.z > m->joints[bi].bb.max.z)
				m->joints[bi].bb.max.z = pos.z;

			if (pos.x < m->joints[bi].bb.min.x)
				m->joints[bi].bb.min.x = pos.x;
			if (pos.y < m->joints[bi].bb.min.y)
				m->joints[bi].bb.min.y = pos.y;
			if (pos.z < m->joints[bi].bb.min.z)
				m->joints[bi].bb.min.z = pos.z;
		}

		//delete what we are done with
		delete[] anims;
		delete[] joints;
		delete[] poses;
		delete[] vas;//vertex arrays
		delete[] baseframe;
		delete[] inversebaseframe;
		delete[] qbaseframe;
		delete[] qinversebaseframe;
		delete[] meshes;
		delete[] framesd;

		logf("vertices: %i, triangles: %i anims: %i joints: %i\n", m->NumVertex, m->NumTriangle, header.num_anims, header.num_joints);
	}
	else
	{
		printf("ERROR: Missing model: %s\n", path);
	}
}

#include "Util\Profile.h"
std::vector<ModelData::OutVert> ModelData::DoDecal(Joint* bone, const Vec3& direction, const Vec3& origin)
{
	PROFILE("DoDecal");
	//crap I need to transform here first
	std::vector<OutVert> output;
	float size = 1.0f;
	int bindex = this->GetBone(bone->name);
	for (int i = 0; i < this->NumTriangle; i++)
	{
		//lets just cheat for now, just return triangles that we hit	
		int vindex = this->TriangleArray[i].Vertex[0] * 4;
		if (this->blendindices[vindex] == bindex
			|| this->blendindices[vindex + 1] == bindex
			|| this->blendindices[vindex + 2] == bindex
			|| this->blendindices[vindex + 3] == bindex)
		{
			Vec3 t1(&this->VertexArray[this->TriangleArray[i].Vertex[0]].X);
			Vec3 t2(&this->VertexArray[this->TriangleArray[i].Vertex[1]].X);
			Vec3 t3(&this->VertexArray[this->TriangleArray[i].Vertex[2]].X);

			//renderer->DebugDrawOBB(OBB(AABB(), Matrix4::Identity));
			//printf("found possible vertex");

			Vec3 normal = (t1 - t2).cross(t1 - t3);

			//get normal of triangle
			//make a stupid triangle here

			if (normal.dot(direction) > 0)
				continue;

			OutVert v;
			v.normal = normal;
			v.blendindex[0] = bindex;
			v.blendweight = 0x000000FF;

			Vec3 offset = normal.getnormal()*-0.05f;
			v.uv = Vec2(0, 0);
			v.pos = t1 + offset;
			output.push_back(v);
			v.uv = Vec2(0, 1);
			v.pos = t2 + offset;
			output.push_back(v);
			v.uv = Vec2(1, 1);
			v.pos = t3 + offset;
			output.push_back(v);
			//ok, lets finish something with this, for now lets just find the plane of the triangle and construct a stupid square
		}
	}

	return output;
}
