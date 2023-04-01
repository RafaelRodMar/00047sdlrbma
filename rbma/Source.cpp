#include<SDL.h>
#include<random>
#include<time.h>
#include<stack>

SDL_Window* g_pWindow = 0;
SDL_Renderer* g_pRenderer = 0;

// keyboard specific
const Uint8* m_keystates;

bool isKeyDown(SDL_Scancode key)
{
	if (m_keystates != 0)
	{
		if (m_keystates[key] == 1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char* args[])
{
	srand(time(nullptr));

	// initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) >= 0)
	{
		// if succeeded create our window
		g_pWindow = SDL_CreateWindow("Recursive backtracker maze algorithm",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			640, 480,
			SDL_WINDOW_SHOWN);
		// if the window creation succeeded create our renderer
		if (g_pWindow != 0)
		{
			g_pRenderer = SDL_CreateRenderer(g_pWindow, -1, 0);
		}
	}
	else
	{
		return 1; // sdl could not initialize
	}

	bool running = true;
	Uint32 frameStart, frameTime;

	//maze things
	int mazeWidth, mazeHeight;
	int cellWidth, cellHeight;
	int *maze;

	//bit fields
	enum
	{
		CELL_PATH_N = 0x01,
		CELL_PATH_E = 0x02,
		CELL_PATH_S = 0x04,
		CELL_PATH_W = 0x08,
		CELL_VISITED = 0x10
	};

	//algorithm things
	int visitedCells;
	std::stack<std::pair<int, int>> pila; //x,y pairs
	int pathWidth;

	//initialization:
	//maze parameters
	mazeWidth = 16;
	mazeHeight = 12;
	cellWidth = 10;
	cellHeight = 10;
	maze = new int[mazeWidth * mazeHeight];
	memset(maze, 0x00, mazeWidth * mazeHeight * sizeof(int));
	pathWidth = 3;

	//choose starting cell
	int x = rand() % mazeWidth;
	int y = rand() % mazeHeight;
	pila.push(std::make_pair(x, y));
	maze[y*mazeWidth + x] = CELL_VISITED;
	visitedCells = 1;
	//end initialization.

	while (running) {
		frameStart = SDL_GetTicks();

		//handle events
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = false;
				break;

			case SDL_KEYDOWN:
				m_keystates = SDL_GetKeyboardState(0);
				break;

			case SDL_KEYUP:
				m_keystates = SDL_GetKeyboardState(0);
				break;

			default:
				break;
			}
		}

		if (isKeyDown(SDL_SCANCODE_ESCAPE)) running = false;

		//update
		//lambda function to calculate index in a readable way
		auto offset = [&](int x, int y)
		{
			return (pila.top().second + y) * mazeWidth + (pila.top().first + x);
		};

		//the maze algorithm
		if (visitedCells < mazeWidth * mazeHeight)
		{
			//create a set of unvisited neighbours
			std::vector<int> neighbours;

			//north
			if (pila.top().second > 0 && (maze[offset(0, -1)] & CELL_VISITED) == 0)
				neighbours.push_back(0);
			//east
			if (pila.top().first < mazeWidth - 1 && (maze[offset(1, 0)] & CELL_VISITED) == 0)
				neighbours.push_back(1);
			//south
			if (pila.top().second < mazeHeight - 1 && (maze[offset(0, 1)] & CELL_VISITED) == 0)
				neighbours.push_back(2);
			//west
			if (pila.top().first > 0 && (maze[offset(-1, 0)] & CELL_VISITED) == 0)
				neighbours.push_back(3);

			//are there any neighbours available?
			if (!neighbours.empty())
			{
				//choose one available neighbour at random
				int next_cell_dir = neighbours[rand() % neighbours.size()];

				//create a path beween the neighbour and the current cell
				switch (next_cell_dir)
				{
				case 0: //north
					maze[offset(0, -1)] |= CELL_VISITED | CELL_PATH_S;
					maze[offset(0, 0)] |= CELL_PATH_N;
					pila.push(std::make_pair((pila.top().first + 0), (pila.top().second - 1)));
					break;
				case 1: //east
					maze[offset(1, 0)] |= CELL_VISITED | CELL_PATH_W;
					maze[offset(0, 0)] |= CELL_PATH_E;
					pila.push(std::make_pair((pila.top().first + 1), (pila.top().second + 0)));
					break;
				case 2: //south
					maze[offset(0, 1)] |= CELL_VISITED | CELL_PATH_N;
					maze[offset(0, 0)] |= CELL_PATH_S;
					pila.push(std::make_pair((pila.top().first + 0), (pila.top().second + 1)));
					break;
				case 3: //west
					maze[offset(-1, 0)] |= CELL_VISITED | CELL_PATH_E;
					maze[offset(0, 0)] |= CELL_PATH_W;
					pila.push(std::make_pair((pila.top().first - 1), (pila.top().second + 0)));
					break;
				default:
					break;
				}

				visitedCells++;
			}
			else
			{
				//no available neighbours so backtrack!
				pila.pop();
			}
		}


		//render
		SDL_SetRenderDrawColor(g_pRenderer, 0, 0, 0, 255);
		// clear the window to black
		SDL_RenderClear(g_pRenderer);

		//draw the actual state of the maze
		for (int xm = 0; xm < mazeWidth; xm++) {
			for (int ym = 0; ym < mazeHeight; ym++) {
				//each cell is inflated by pathWidth, so fill it in
				for (int py = 0; py < pathWidth; py++) {
					for (int px = 0; px < pathWidth; px++) {
						if (maze[ym*mazeWidth + xm] & CELL_VISITED)
						{
							Uint8 cr = 255, cg = 255, cb = 255;
							SDL_SetRenderDrawColor(g_pRenderer, cr, cg, cb, 255);
							SDL_Rect rect;
							rect.x = xm * (pathWidth + 1) + px;
							rect.y = ym * (pathWidth + 1) + py;
							rect.x *= cellWidth;
							rect.y *= cellHeight;
							rect.h = cellHeight;
							rect.w = cellWidth;
							SDL_RenderFillRect(g_pRenderer, &rect);
						}
						else
						{
							Uint8 cr = 0, cg = 0, cb = 255;
							SDL_SetRenderDrawColor(g_pRenderer, cr, cg, cb, 255);
							SDL_Rect rect;
							rect.x = xm * (pathWidth + 1) + px;
							rect.y = ym * (pathWidth + 1) + py;
							rect.x *= cellWidth;
							rect.y *= cellHeight;
							rect.h = cellHeight;
							rect.w = cellWidth;
							SDL_RenderFillRect(g_pRenderer, &rect);
						}
					}
				}

				//draw passageways between cells
				//for (int p = 0; p < pathWidth; p++) {
					if (maze[ym * mazeWidth + xm] & CELL_PATH_S)
					{
						Uint8 cr = 255, cg = 255, cb = 255;
						SDL_SetRenderDrawColor(g_pRenderer, cr, cg, cb, 255);
						SDL_Rect rect;
						rect.x = xm * (pathWidth + 1);
						rect.y = ym * (pathWidth + 1) + pathWidth;
						rect.x *= cellWidth;
						rect.y *= cellHeight;
						rect.h = pathWidth * cellHeight;
						rect.w = cellWidth * pathWidth;
						SDL_RenderFillRect(g_pRenderer, &rect);
					}

					if (maze[ym*mazeWidth + xm] & CELL_PATH_E)
					{
						Uint8 cr = 255, cg = 255, cb = 255;
						SDL_SetRenderDrawColor(g_pRenderer, cr, cg, cb, 255);
						SDL_Rect rect;
						rect.x = xm * (pathWidth + 1) + pathWidth;
						rect.y = ym * (pathWidth + 1);
						rect.x *= cellWidth;
						rect.y *= cellHeight;
						rect.w = cellWidth * pathWidth;
						rect.h = cellHeight * pathWidth;
						SDL_RenderFillRect(g_pRenderer, &rect);
					}
				//}
			}
		}

		//draw Unit - the top of the stack
		for (int py = 0; py < pathWidth; py++) {
			for (int px = 0; px < pathWidth; px++) {
				Uint8 cr = 0, cg = 255, cb = 0;
				SDL_SetRenderDrawColor(g_pRenderer, cr, cg, cb, 255);
				SDL_Rect rect;
				rect.x = pila.top().first * (pathWidth+1) + px;
				rect.y = pila.top().second * (pathWidth+1) + py;
				rect.x *= cellWidth;
				rect.y *= cellHeight;
				rect.h = cellHeight;
				rect.w = cellWidth;
				SDL_RenderFillRect(g_pRenderer, &rect);
			}
		}
		//end drawing the actual state of the maze

		// show the window
		SDL_RenderPresent(g_pRenderer);

		frameTime = SDL_GetTicks() - frameStart;

		if (frameTime < DELAY_TIME)
		{
			SDL_Delay((int)(DELAY_TIME - frameTime));
		}
	}

	// clean up SDL
	SDL_DestroyWindow(g_pWindow);
	SDL_DestroyRenderer(g_pRenderer);
	SDL_Quit();
	return 0;
}