#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>  
#include <stdio.h>
#include <stdlib.h>

#define MY_NUM_THREADS 4

typedef struct
{       
        char ***      old_m;
        char ***      new_m;
		size_t      start;
		size_t      end;
		size_t      columns;
        size_t      rows;
} my_struct;

pthread_barrier_t g_Barrier; 

char ** old_m;
char ** new_m;

char alive, dead;

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void createMatrix(char *** matrix, size_t rows, size_t columns)
{
	*matrix = (char**)calloc(rows, sizeof(char*));

	for(size_t i = 0; i < rows; i++) 
	{
		(*matrix)[i] = (char *)calloc(columns, sizeof(char));
	}
}

void fillMatrix(char *** matrix, size_t rows, size_t columns)
{
	srand(time(NULL));
	for (size_t i = 0; i < rows; ++i)
	{
		for (size_t j = 0; j < columns; ++j)
		{	
			if ((int)(rand() % 2))
			{
				(*matrix)[i][j] = alive;
			}
			else
			{
				(*matrix)[i][j] = dead;
			}
		}
	}
}

void print_matrix(char *** matrix, size_t rows, size_t columns)
{
    for(size_t i = 0; i < rows; ++i)
	{
		for (size_t j = 0; j < columns; ++j)
			printf("%2c", (*matrix)[i][j]);
		printf("\n");
	}
}

void *MyThreadProc(void *data) 
{       
        my_struct *d = (my_struct *)data;
        char ***old_matrix = d->old_m;
        char ***new_matrix = d->new_m;
	for(int i = d->start; i < d->end; ++i)
	{	
		for (int j = 0; j < d->columns; ++j)
                {
                        int cells = 0;
                        for (int y = i-1; y <= i+1; ++y)
                        {
                            for (int k = j-1; k <= j+1; ++k)
                            {
                                if ((y==i && k==j) || y < 0 || k < 0 || y >= d->rows || k >= d->columns)
                                {
                                    continue;
                                }
                                if ((*old_matrix)[y][k] == alive)
				{
                                    cells += 1;
				}
                            }
                        }
                        if ((*old_matrix)[i][j] == dead)
                        {
                            if (cells > 3)
                            {
                                (*new_matrix)[i][j] = alive;
                            }
                            else
                            {
                                (*new_matrix)[i][j] = dead;
                            }
                        }
                        
                        if ((*old_matrix)[i][j] == alive)
                        {
			    
                            if (cells < 2)
                            {
                                (*new_matrix)[i][j] = dead;
                            }
                            if (cells > 3)
                            {
                                (*new_matrix)[i][j] = dead;
                            }
                            if (cells == 2 || cells == 3)
                            {
                                (*new_matrix)[i][j] = alive;
                            }
                        }
                }
	}
	pthread_barrier_wait(&g_Barrier); 
	return 0;
}

int main(int argc, char *argv[])
{       
	alive = '#';
	dead = ' ';
	double res;
        int i = 0;
	size_t rows = 60;
	size_t columns = 90;
        my_struct *data = (my_struct*)malloc(MY_NUM_THREADS * sizeof(my_struct));
	
	createMatrix(&old_m, rows, columns);
	fillMatrix(&old_m, rows, columns);
	print_matrix(&old_m, rows, columns);
        
        createMatrix(&new_m, rows, columns);
	fillMatrix(&new_m, rows, columns);
        
        pthread_t threads[MY_NUM_THREADS];
        pthread_barrier_init(&g_Barrier, NULL, MY_NUM_THREADS);
	for (i = 0; i < MY_NUM_THREADS; ++i)
        {       
                data[i].columns = columns;
                data[i].rows = rows;
                data[i].start = i * (int)(rows / MY_NUM_THREADS);
                data[i].end = (i+1) * (int)(rows / MY_NUM_THREADS);
	}
	for (int n = 0; n < 100; ++n)
	{	
		for (i = 0; i < MY_NUM_THREADS; ++i)
		{ 	
			if (n % 2 == 0)
			{
				data[i].old_m = &old_m;
				data[i].new_m = &new_m;
			}
			else
			{
				data[i].old_m = &new_m;
				data[i].new_m = &old_m;
			}
			pthread_create(&threads[i], NULL, MyThreadProc, (void *)(&data[i]));
		}
		for (i = 0; i < MY_NUM_THREADS; i++)            
			pthread_join(threads[i], NULL);  
		printf("\n");
		if (n % 2 == 0)
		{
			print_matrix(&new_m, rows, columns);
		}
		else
		{
			print_matrix(&old_m, rows, columns);
		}
		msleep(1000);
	}
	pthread_barrier_destroy(&g_Barrier);
	pthread_exit(NULL);
}
