// 3D_Engine_3.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
using namespace std;

#include <stdio.h>
#include <Windows.h>

int nScreenWidth = 140;			        // Размер экрана консоли X (столбцы)
int nScreenHeight = 60;			        // Размер экрана консоли Y (строки)
int nMapWidth = 40;			   	        // Размер окружения
int nMapHeight = 40;

float fPlayerX = 1.0f;		   	        // Стартовая позиция игрока
float fPlayerY = 1.0f;
float fPlayerA = 0.0f;			        // Начальный угол вращения игрока
float fFOV = 3.14159f / 3.0f;	        // Поле зрения
float fDepth = 30.0f;			        // Максимальное расстояние отрисовки
float fSpeed = 5.0f;			        // Скорость ходьбы

int main()
{
	// Создание буфера экрана

	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Создание карты пространства # = блок стены, . = пустое пространство

	wstring map;
	map += L"########################################";
	map += L"#......................................#";
	map += L"###...........#..........#...........###";
	map += L"#.....##......#..........#......##.....#";
	map += L"###..................................###";
	map += L"#.............####....####.............#";
	map += L"#....####.....#..#....#..#.....####....#";
	map += L"#....#..########.##..##.########..#....#";
	map += L"#....#...........#....#...........#....#";
	map += L"#....####.....####....####.....####....#";
	map += L"#.....#..........................#.....#";
	map += L"#.....#..........................#.....#";
	map += L"#....#.....##....#....#....##.....#....#";
	map += L"#...#..............................#...#";
	map += L"#...#.......####........####.......#...#";
	map += L"#....#.....#...####..####...#.....#....#";
	map += L"#.....#.....##.#........#.##.....#.....#";
	map += L"#.....#......#.....##.....#......#.....#";
	map += L"#....####....#...#.#.##...#....#.##....#";
	map += L"#...#...#....#...#....#...#....#.#.#...#";
	map += L"#...#.#.#....#...#....#...#....#...#...#";
	map += L"#....##.#....#...##.#.#...#....####....#";
	map += L"#.....#......#.....##.....#......#.....#";
	map += L"#.....#.....##.#........#.##.....#.....#";
	map += L"#....#.....#...####..####...#.....#....#";
	map += L"#...#.......####........####.......#...#";
	map += L"#...#..............................#...#";
	map += L"#....#.....##....#....#....##.....#....#";
	map += L"#.....#..........................#.....#";
	map += L"#.....#..........................#.....#";
	map += L"#....####.....####....####.....####....#";
	map += L"#....#...........#....#...........#....#";
	map += L"#....#..########.##..##.########..#....#";
	map += L"#....####.....#..#....#..#.....####....#";
	map += L"#.............####....####.............#";
	map += L"###..................................###";
	map += L"#.....##......#..........#......##.....#";
	map += L"###...........#..........#...........###";
	map += L"#......................................#";
	map += L"########################################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	while (1)
	{
		// Нам понадобится разница во времени для каждого кадра,
		// чтобы рассчитать изменение скорости движения, чтобы
		// гарантировать постоянное движение, поскольку трассировка
		// лучей не является детерминированной

		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();


		// Обработка вращения против часовой стрелки

		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;

		// Обработка вращения по часовой стрелке

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (fSpeed * 0.75f) * fElapsedTime;

		// Обработка движения вперёд и столкновения

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;;
			fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
				fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;;
			}
		}

		// Обработка движения назад и столкновения

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
			fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;;
				fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;;
			}
		}

		for (int x = 0; x < nScreenWidth; x++)
		{
			// Для каждого столбца вычисляется угол проецирования луча в окружающее пространство

			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			// Нахождение расстояния до стены

			float fStepSize = 0.1f;		                // Увеличение размера для лучей, уменьшение
			float fDistanceToWall = 0.0f;               // размера для увеличения разрешения

			bool bHitWall = false;		                // Устанавливается, когда луч попадает в стену
			bool bBoundary = false;		                // Устанавливается, когда луч достигает границы между двумя блоками стен

			float fEyeX = sinf(fRayAngle);              // Единичный вектор для луча в пространстве около игрока
			float fEyeY = cosf(fRayAngle);

			// Постепенно направленный луч от игрока по углу
			// луча, проверка на пересечение с блоком

			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += fStepSize;
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				// Проверка, выходит ли луч за пределы

				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;			        // Расстояние устанавливается на максимальную глубину
					fDistanceToWall = fDepth;
				}
				else
				{
					// Луч находится внутри, поэтому проверяем, не
					// является ли лучевая ячейка блоком стены

					if (map.c_str()[nTestX * nMapWidth + nTestY] == '#')
					{
						// Луч падает на стену

						bHitWall = true;

						// Чтобы выделить границы плитки, направьте луч из каждого
						// угла плитки на игрока. Чем больше этот луч совпадает с лучом
						// рендеринга, тем ближе мы к границе плитки, которую мы
						// закрасим, чтобы добавить детали к стенам

						vector<pair<float, float>> p;

						// Проверяем каждый угол поражённой плитки, запоминая расстояние
						// от игрока и вычисленное скалярное произведение двух лучей

						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								// Угол от угла между стен к глазу

								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}

						// Сортировка Пар от ближайшей к самой дальней

						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

						// Первые два/три ближайшие (мы никогда не увидим всех четырёх)

						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}

			// Рассчёт расстояния до потолка и пола

			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			// Затенение стены в зависимости от расстояния

			short nShade = ' ';
			if (fDistanceToWall <= fDepth / 4.0f)			nShade = 0x2588;	        // Очень близко
			else if (fDistanceToWall < fDepth / 3.0f)		nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)		nShade = 0x2592;
			else if (fDistanceToWall < fDepth)				nShade = 0x2591;
			else											nShade = ' ';		        // Слишком далеко

			if (bBoundary)	                            	nShade = ' ';               // Уберите это))

			for (int y = 0; y < nScreenHeight; y++)
			{
				// Каждый ряд

				if (y <= nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else                                                                    // Пол
				{
					// Затенение пола в зависимости от расстояния

					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';
					screen[y * nScreenWidth + x] = nShade;
				}
			}
		}

		// Информация на экране

		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		// Карта окружения на экране

		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';

		// Рамка дисплея

		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
