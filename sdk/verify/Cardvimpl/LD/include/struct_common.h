#pragma once
#include<stdio.h>
typedef struct Detectbox_
{
	float x1;
	float y1;
	float x2;
	float y2;
	int classid;
	float score;
	bool current = false;
	int flag = 0;
	Detectbox_ operator=(const Detectbox_& data)
	{
		if (this == &data)
		{
			return *this;
		}
		this->x1 = data.x1;
		this->y1 = data.y1;
		this->x2 = data.x2;
		this->y2 = data.y2;
		this->classid = data.classid;
		this->score = data.score;
		this->flag = data.flag;
		this->current = data.current;
		return *this;

	};
}Detectbox;