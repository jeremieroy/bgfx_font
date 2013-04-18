/* Copyright 2013 Jeremie Roy. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#include <bgfx.h> 
#include <assert.h>
#include "RectanglePacker.h"

struct AtlasRegion
{
	enum Type
	{
		TYPE_GRAY = 1, // 1 component
		TYPE_BGRA8 = 4  // 4 components
	};	

	uint16_t x, y;
	uint16_t width, height;
	uint32_t mask; //encode the region type, the face index and the component index in case of a gray region

	Type getType()const           { return (Type) ((mask >> 0) & 0x0000000F); }
	uint32_t getFaceIndex()const  { return         (mask >> 4) & 0x0000000F; }
	uint32_t getComponentIndex()const { return         (mask >> 8) & 0x0000000F; }
	void setMask(Type type, uint32_t faceIndex, uint32_t componentIndex) { mask = (componentIndex << 8) +  (faceIndex << 4) + (uint32_t)type; }
};

class Atlas
{
public:
	/// create an empty dynamic atlas (region can be updated and added)
	/// @param textureSize an atlas creates a texture cube of 6 faces with size equal to (textureSize*textureSize * sizeof(RGBA))
	/// @param maxRegionCount maximum number of region allowed in the atlas	
	Atlas(uint16_t textureSize, uint16_t _maxRegionsCount = 4096);
		
	/// initialize a static atlas with serialized data	(region can be updated but not added)
	/// @param textureSize an atlas creates a texture cube of 6 faces with size equal to (textureSize*textureSize * sizeof(RGBA))
	/// @param textureBuffer buffer of size 6*textureSize*textureSize*sizeof(uint32_t) (will be copied)
	/// @param regionCount number of region in the Atlas
	/// @param regionBuffer buffer containing the region (will be copied)
	/// @param maxRegionCount maximum number of region allowed in the atlas
	Atlas(uint16_t textureSize, const uint8_t * textureBuffer, uint16_t regionCount, const uint8_t* regionBuffer, uint16_t maxRegionsCount = 4096);
	~Atlas();
	
	/// add a region to the atlas, and copy the content of mem to the underlying texture
	uint16_t addRegion(uint16_t width, uint16_t height, const uint8_t* bitmapBuffer, AtlasRegion::Type type = AtlasRegion::TYPE_BGRA8);

	/// update a preallocated region
	void updateRegion(const AtlasRegion& region, const uint8_t* bitmapBuffer);

	/// Pack the UV coordinates of the four corners of a region to a vertex buffer using the supplied vertex format.
	/// v0 -- v3
	/// |     |     encoded in that order:  v0,v1,v2,v3
	/// v1 -- v2
	/// @remark the UV are four signed short normalized components.
	/// @remark the x,y,z components encode cube uv coordinates. The w component encode the color channel if any.	
	/// @param handle handle to the region we are interested in
	/// @param vertexBuffer address of the first vertex we want to update. Must be valid up to vertexBuffer + offset + 3*stride + 4*sizeof(int16_t), which means the buffer must contains at least 4 vertex includind the first.
	/// @param offset byte offset to the first uv coordinate of the vertex in the buffer
	/// @param stride stride between tho UV coordinates, usually size of a Vertex.
	void packUV( uint16_t regionHandle, uint8_t* vertexBuffer, uint32_t offset, uint32_t stride );
	void packUV( const AtlasRegion& region, uint8_t* vertexBuffer, uint32_t offset, uint32_t stride );
	
	/// Same as packUV but pack a whole face of the atlas cube, mostly used for debugging and visualizing atlas
	void packFaceLayerUV(uint32_t idx, uint8_t* vertexBuffer, uint32_t offset, uint32_t stride )
	{
		packUV(m_layers[idx].faceRegion, vertexBuffer, offset, stride);
	}	

	/// Pack the vertex index of the region as 2 quad into an index buffer
	void packIndex(uint16_t* indexBuffer, uint32_t startIndex, uint32_t startVertex )
	{
		indexBuffer[startIndex+0] = startVertex+0;
		indexBuffer[startIndex+1] = startVertex+1;
		indexBuffer[startIndex+2] = startVertex+2;
		indexBuffer[startIndex+3] = startVertex+0;
		indexBuffer[startIndex+4] = startVertex+2;
		indexBuffer[startIndex+5] = startVertex+3;
	}

	/// return the TextureHandle (cube) of the atlas
	bgfx::TextureHandle getTextureHandle() const { return m_textureHandle; }

	//retrieve a region info
	const AtlasRegion& getRegion(uint16_t handle) const { return m_regions[handle]; }

	/// retrieve the usage ratio of the atlas
	float getUsageRatio() const { return 0.0f; }

	/// retrieve the numbers of region in the atlas
	uint16_t getRegionCount() const { return m_regionCount; }

	/// retrieve a pointer to the region buffer (in order to serialize it)
	const AtlasRegion* getRegionBuffer() const { return m_regions; }
	
	/// retrieve the byte size of the texture
	uint32_t getTextureBufferSize() const { return 6*m_textureSize*m_textureSize*4; }

	/// retrieve the mirrored texture buffer (to serialize it)
	const uint8_t* getTextureBuffer() const { return m_textureBuffer; }

private:

	void writeUV( uint8_t* vertexBuffer, int16_t x, int16_t y, int16_t z, int16_t w) 
	{
		((uint16_t*) vertexBuffer)[0] = x;
		((uint16_t*) vertexBuffer)[1] = y; 
		((uint16_t*) vertexBuffer)[2] = z; 
		((uint16_t*) vertexBuffer)[3] = w; 
	}
	struct PackedLayer
	{
		bgfx_font::RectanglePacker packer;
		AtlasRegion faceRegion;
	};
	PackedLayer m_layers[24];
	uint32_t m_usedLayers;
	uint32_t m_usedFaces;

	bgfx::TextureHandle m_textureHandle;
	uint16_t m_textureSize;

	uint16_t m_regionCount;
	uint16_t m_maxRegionCount;
	
	AtlasRegion* m_regions;	
	uint8_t* m_textureBuffer;

};

inline Atlas::Atlas(uint16_t textureSize, uint16_t maxRegionsCount )
{
	assert(textureSize >= 64 && textureSize <= 4096 && "suspicious texture size" );
	assert(maxRegionsCount >= 64 && maxRegionsCount <= 32000 && "suspicious regions count" );
	
	for(int i=0; i<24;++i)
	{
		m_layers[i].packer.init(textureSize, textureSize);		
	}
	m_usedLayers = 0;
	m_usedFaces = 0;

	m_textureSize = textureSize;
	m_regionCount = 0;
	m_maxRegionCount = maxRegionsCount;
	m_regions = new AtlasRegion[maxRegionsCount];
	m_textureBuffer = new uint8_t[ textureSize * textureSize * 6 * 4 ];
	memset(m_textureBuffer, 0,  textureSize * textureSize * 6 * 4);
	//BGFX_TEXTURE_MIN_POINT|BGFX_TEXTURE_MAG_POINT|BGFX_TEXTURE_MIP_POINT;
	//BGFX_TEXTURE_MIN_ANISOTROPIC|BGFX_TEXTURE_MAG_ANISOTROPIC|BGFX_TEXTURE_MIP_POINT
	//BGFX_TEXTURE_U_CLAMP|BGFX_TEXTURE_V_CLAMP
	uint32_t flags = 0;// BGFX_TEXTURE_MIN_ANISOTROPIC|BGFX_TEXTURE_MAG_ANISOTROPIC|BGFX_TEXTURE_MIP_POINT;

	//Uncomment this to debug atlas
	//const bgfx::Memory* mem = bgfx::alloc(textureSize*textureSize * 6 * 4);
	//memset(mem->data, 255, mem->size);	
	const bgfx::Memory* mem = NULL;	
	m_textureHandle = bgfx::createTextureCube(6
			, textureSize
			, 1
			, bgfx::TextureFormat::BGRA8
			, flags
			,mem
			);
}

inline Atlas::Atlas(uint16_t textureSize, const uint8_t* textureBuffer , uint16_t regionCount, const uint8_t* regionBuffer, uint16_t maxRegionsCount)
{
	assert(regionCount <= 64 && maxRegionsCount <= 4096);
	//layers are frozen
	m_usedLayers = 24;
	m_usedFaces = 6;

	m_textureSize = textureSize;
	m_regionCount = regionCount;
	//regions are frozen
	m_maxRegionCount = regionCount;
	m_regions = new AtlasRegion[regionCount];
	m_textureBuffer = new uint8_t[getTextureBufferSize()];
	
	//BGFX_TEXTURE_MIN_POINT|BGFX_TEXTURE_MAG_POINT|BGFX_TEXTURE_MIP_POINT;
	//BGFX_TEXTURE_MIN_ANISOTROPIC|BGFX_TEXTURE_MAG_ANISOTROPIC|BGFX_TEXTURE_MIP_POINT
	//BGFX_TEXTURE_U_CLAMP|BGFX_TEXTURE_V_CLAMP
	uint32_t flags = 0;//BGFX_TEXTURE_MIN_ANISOTROPIC|BGFX_TEXTURE_MAG_ANISOTROPIC|BGFX_TEXTURE_MIP_POINT;
	memcpy(m_regions, regionBuffer, regionCount * sizeof(AtlasRegion));	
	memcpy(m_textureBuffer, textureBuffer, getTextureBufferSize());

	m_textureHandle = bgfx::createTextureCube(6
			, textureSize
			, 1
			, bgfx::TextureFormat::BGRA8
			, flags
			, bgfx::makeRef(m_textureBuffer, getTextureBufferSize())
			);
}

inline Atlas::~Atlas()
{
	delete[] m_regions;
	delete[] m_textureBuffer;
}

inline uint16_t Atlas::addRegion(uint16_t width, uint16_t height, const uint8_t* bitmapBuffer,  AtlasRegion::Type type)
{
	if (m_regionCount >= m_maxRegionCount)
	{
		return UINT16_MAX;
	}
	
	uint16_t x,y;
	// We want each bitmap to be separated by at least one black pixel
	// TODO manage mipmaps
	uint32_t idx = 0;
	while(idx<m_usedLayers)
	{
		if(m_layers[idx].faceRegion.getType() == type)
		{
			if(m_layers[idx].packer.addRectangle(width+1,height+1,x,y)) break;			
		}
		idx++;
	}

	if(idx >= m_usedLayers)
	{
		//do we have still room to add layers ?
		if( (idx + type) > 24 || m_usedFaces>=6)
		{
				return UINT16_MAX;
		}		
		//create new layers
		for(int i=0; i < type;++i)
		{
			m_layers[idx+i].faceRegion.setMask(type, m_usedFaces, i);			
		}
		m_usedLayers += type;
		m_usedFaces++;


		//add it to the created layer
		if(!m_layers[idx].packer.addRectangle(width+1,height+1,x,y))
		{
			return UINT16_MAX;
		}
	}
	
	AtlasRegion& region = m_regions[m_regionCount];
	region.x = x;
	region.y = y;
	region.width = width;
	region.height = height;
	region.mask = m_layers[idx].faceRegion.mask;

	updateRegion(region, bitmapBuffer);
	return m_regionCount++;
}

inline void Atlas::updateRegion(const AtlasRegion& region, const uint8_t* bitmapBuffer)
{	
	const bgfx::Memory* mem = bgfx::alloc(region.width * region.height * 4);
	//BAD!
	memset(mem->data,0, mem->size);
	if(region.getType() == AtlasRegion::TYPE_BGRA8)
	{	
		const uint8_t* inLineBuffer = bitmapBuffer;
		uint8_t* outLineBuffer = m_textureBuffer + region.getFaceIndex() * (m_textureSize*m_textureSize*4) + (((region.y *m_textureSize)+region.x)*4);

		//update the cpu buffer
		for(int y = 0; y < region.height; ++y)
		{
			memcpy(outLineBuffer, inLineBuffer, region.width * 4);
			inLineBuffer += region.width*4;
			outLineBuffer += m_textureSize*4;
		}
		//update the GPU buffer
		memcpy(mem->data, bitmapBuffer, mem->size);
	}else
	{
		uint32_t layer = region.getComponentIndex();
		uint32_t face = region.getFaceIndex();
		const uint8_t* inLineBuffer = bitmapBuffer;
		uint8_t* outLineBuffer = (m_textureBuffer + region.getFaceIndex() * (m_textureSize*m_textureSize*4) + (((region.y *m_textureSize)+region.x)*4));
		
		//update the cpu buffer
		for(int y = 0; y<region.height; ++y)
		{
			for(int x = 0; x<region.width; ++x)
			{
				outLineBuffer[(x*4) + layer] = inLineBuffer[x];
			}
			//update the GPU buffer
			memcpy(mem->data + y*region.width*4, outLineBuffer, region.width*4);
			inLineBuffer += region.width;
			outLineBuffer +=  m_textureSize*4;
		}
	}
	bgfx::updateTextureCube(m_textureHandle, (uint8_t)region.getFaceIndex(), 0, region.x, region.y, region.width, region.height, mem);		
}

inline void Atlas::packUV( uint16_t handle, uint8_t* vertexBuffer, uint32_t offset, uint32_t stride )
{
	const AtlasRegion& region = m_regions[handle];
	packUV(region, vertexBuffer, offset, stride);
}

inline void Atlas::packUV( const AtlasRegion& region, uint8_t* vertexBuffer, uint32_t offset, uint32_t stride )
{
	float texMult = 65535.0f / ((float)(m_textureSize));
	static const int16_t minVal = -32768;
	static const int16_t maxVal = 32767;
	
	int16_t x0 = (int16_t)(region.x * texMult)-32768;
	int16_t y0 = (int16_t)(region.y * texMult)-32768;
	int16_t x1 = (int16_t)((region.x + region.width)* texMult)-32768;
	int16_t y1 = (int16_t)((region.y + region.height)* texMult)-32768;
	int16_t w =  (int16_t) ((32767.0f/4.0f) * region.getComponentIndex());

	vertexBuffer+=offset;
	switch(region.getFaceIndex())
	{
	case 0: // +X
		x0= -x0; 
		x1= -x1;
		y0= -y0; 
		y1= -y1;
		writeUV(vertexBuffer, maxVal, y0, x0, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, maxVal, y1, x0, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, maxVal, y1, x1, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, maxVal, y0, x1, w); vertexBuffer+=stride;
		break;
	case 1: // -X		
		y0= -y0; 
		y1= -y1;
		writeUV(vertexBuffer, minVal, y0, x0, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, minVal, y1, x0, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, minVal, y1, x1, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, minVal, y0, x1, w); vertexBuffer+=stride;	
		break;
	case 2: // +Y		
		writeUV(vertexBuffer, x0, maxVal, y0, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x0, maxVal, y1, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x1, maxVal, y1, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x1, maxVal, y0, w); vertexBuffer+=stride;
		break;
	case 3: // -Y
		y0= -y0; 
		y1= -y1;
		writeUV(vertexBuffer, x0, minVal, y0, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x0, minVal, y1, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x1, minVal, y1, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x1, minVal, y0, w); vertexBuffer+=stride;
		break;
	case 4: // +Z
		y0= -y0; 
		y1= -y1;
		writeUV(vertexBuffer, x0, y0, maxVal, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x0, y1, maxVal, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x1, y1, maxVal, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x1, y0, maxVal, w); vertexBuffer+=stride;
		break;
	case 5: // -Z
		x0= -x0; 
		x1= -x1;
		y0= -y0; 
		y1= -y1;
		writeUV(vertexBuffer, x0, y0, minVal, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x0, y1, minVal, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x1, y1, minVal, w); vertexBuffer+=stride;
		writeUV(vertexBuffer, x1, y0, minVal, w); vertexBuffer+=stride;
		break;
	}
}