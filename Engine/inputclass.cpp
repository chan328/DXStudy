///////////////////////////////
// Filename : inputclass.cpp //
///////////////////////////////

#include "inputclass.h"

InputClass::InputClass()
{

}

InputClass::InputClass(const InputClass& other)
{

}

InputClass::~InputClass()
{

}

void InputClass::Initialize()
{
	int i;

	// 모든 키들을 눌리지 않은 상태로 초기화합니다.
	for (i = 0; i < 256; i++)
	{
		m_keys[i] = false;
	}

	return;
}

void InputClass::KeyDown(unsigned int input)
{
	// 키가 눌렸다면 그 상태를 배열에 저장합니다.
	m_keys[input] = true;
	return;
}

void InputClass::KeyUp(unsigned int input)
{
	// 키가 떼어졌다면 그 상태를 배열에 저장합니다.
	m_keys[input] = false;
	return;
}

bool InputClass::IsKeyDown(unsigned int key)
{
	return m_keys[key];
}