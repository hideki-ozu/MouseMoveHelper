#pragma once

#include "resource.h"

#include "shellapi.h"
#include "shellscalingapi.h"
#include <vector>
#include <algorithm>
#include <utility>
#include <gsl/gsl>

#ifdef _DEBUG
#include <string>
#endif

#define	MOVE_LEFT	0x01
#define	MOVE_RIGHT	0x02
#define	MOVE_UP		0x04
#define	MOVE_DOWN	0x08

typedef struct _EDGEDATA
{
	long edge_near;
	long edge_far;
	long translate_range_high;
	long translate_range_low;

	_EDGEDATA() noexcept
	{
		edge_near = 0;
		edge_far = 0;
		translate_range_high = 0;
		translate_range_low = 0;
	}
}EDGEDATA;

LRESULT CALLBACK LowLevelMouseProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);
BOOL CALLBACK InfoEnumProc(HMONITOR hMon, HDC hdcMon, gsl::not_null<LPRECT> lpMon, LPARAM dwData);

void addTaskTrayIcon(HWND hWnd) noexcept;
void delTaskTrayIcon(HWND hWnd) noexcept;
