// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "VertexArray.h"
#include <GL/glew.h>

VertexArray::VertexArray(const float* verts, const int size, unsigned int numVerts, const unsigned int* indices, unsigned int numIndices): mNumVerts(numVerts), mNumIndices(numIndices)
{
	// Create vertex array
	glGenVertexArrays(1, &mVertexArray);
	glBindVertexArray(mVertexArray);
	// Create vertex buffer
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, numVerts * size * sizeof(float), verts, GL_STATIC_DRAW);
	// Create index buffer
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);
	/* Specify the vertex attributes (For now, assume one vertex format)
	NOTE: Position is 3 floats starting at offset 0*/
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * size, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * size, reinterpret_cast<void*>(sizeof(float) * 3));
	// Tell OpenGl we want to create a new Attribute ID=2
	glEnableVertexAttribArray(2);
	// Tell OpenGL we want to create a Vecter3 of floats
	// NOTE: ID, vector size, type, IDK, total size, starting index
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * size, reinterpret_cast<void*>(sizeof(float) * 5));
}

VertexArray::~VertexArray()
{
	glDeleteBuffers(1, &mVertexBuffer);
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteVertexArrays(1, &mVertexArray);
}

void VertexArray::SetActive()
{
	glBindVertexArray(mVertexArray);
}