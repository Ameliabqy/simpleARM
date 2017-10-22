#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

const int maxN = 1000000;
const int window = 10;
const int limit = 30;

float roll[maxN], pitch[maxN], yaw[maxN], pos[maxN][3];
double s_roll[maxN], s_pitch[maxN], s_yaw[maxN];

void fun(int &c, double d, int &b)
{
	if (c == 0)
	{
		if (d > 0.1)
			++c;
		else if (d < -0.1)
			--c;
		b = 0;
	}
	else if (c > 0)
	{
		if (d > 0.1)
			++c;
		else
			b = c = 0;
	}
	else if (c < 0)
	{
		if (d < -0.1)
			--c;
		else
			b = c = 0;
	}
}

int main(int argc, char **argv)
{
	int fd;
    char * myfifo = "pipe_data";
    mkfifo(myfifo, 0666);
    fd = open(myfifo, O_WRONLY);
    FILE* fvel = fdopen(fd, "w");

//	FILE* fvel = fopen("output", "w");

	freopen("rotate_combine.txt", "r", stdin);
	char cs[256];
	for (int i = 0; i < 6; ++i)
		fgets(cs, 256, stdin);
	float k = 1, K = 100, tmp;
	float vx, vy, vz;
	float acc_x, acc_y, acc_z;
	float gyr_x, gyr_y, gyr_z;
	float d_roll, d_pitch, d_yaw, maxZ = 0;
	int c_roll = 0, c_pitch = 0, c_yaw = 0;
	int b_roll = 0, b_pitch = 0, b_yaw = 0;
	int counter, tmp_grab, grab = 0, change = 0;
	
	for (int i = 0; 
			scanf("%d%f%f%f%f%f%f%f%f%f%f\n", &counter,
				&acc_x, &acc_y, &acc_z,
				&gyr_x, &gyr_y, &gyr_z,
				&roll[i], &pitch[i], &yaw[i],
				&tmp) != EOF;
			++i)
	{
		fprintf(stderr, "%d\n", i);

		if (i)
		{
			s_roll[i] = s_roll[i-1] + roll[i];
			s_pitch[i] = s_pitch[i-1] + pitch[i];
			s_yaw[i] = s_pitch[i-1] + yaw[i];
		} else
		{
			s_roll[i] = roll[i];
			s_pitch[i] = pitch[i];
			s_yaw[i] = yaw[i];			
		}

		pos[i][2] = k * (roll[i] - s_roll[i] / (i+1));
		pos[i][0] = k * (pitch[i] - s_pitch[i] / (i+1));
		pos[i][1] = k * (yaw[i] - s_yaw[i] / (i+1));
		if (pos[i][2] > maxZ) maxZ = pos[i][2];
/*
		for (int j = 0; j < 2; ++j)
			fprintf(fpos, "%f\t", pos[i][j]);
		fprintf(fpos, "%f\n", pos[i][2]);
*/
		if (i >= 3)
		{
			d_roll = roll[i] - roll[i-3];
			d_pitch = pitch[i] - pitch[i-3];
			d_yaw = yaw[i] - yaw[i-3];
			fun(c_roll, d_roll, b_roll);
			fun(c_pitch, d_pitch, b_pitch);
			fun(c_yaw, d_yaw, b_yaw);
			b_roll = b_pitch = b_yaw = 0;
			if (i >= window * 2)
			{
				double t1 = (s_roll[i] - s_roll[i-window]) / window - (s_roll[i-window] - s_roll[i-window*2]) / window;
				double t2 = (s_pitch[i] - s_pitch[i-window]) / window - (s_pitch[i-window] - s_pitch[i-window*2]) / window;
				double t3 = (s_yaw[i] - s_yaw[i-window]) / window - (s_yaw[i-window] - s_yaw[i-window*2]) / window;
				if (t1 > limit || t1 < -limit) b_roll = 1;
				if (t2 > limit || t2 < -limit) b_pitch = 1;
				if (t3 > limit || t3 < -limit) b_yaw = 1;
			}
			if (b_roll != 0) vx = (roll[i] - s_roll[i] / (i+1)) * K;
			if (b_pitch != 0) vy = (pitch[i] - s_pitch[i] / (i+1)) * K;
			if (b_yaw != 0) vz = (yaw[i] - s_yaw[i] / (i+1)) * K;
			if ((b_roll && b_pitch && b_yaw) != tmp_grab)
				grab = 1;
			else 
				grab = 0;
			tmp_grab = b_roll && b_pitch && b_yaw;
			fprintf(fvel, "%f ", vx);
			fprintf(fvel, "%f ", vy);
			fprintf(fvel, "%f ", vz);
			for (int j = 0; j < 2; ++j)
				fprintf(fvel, "%f ", pos[i][j]);
			fprintf(fvel, "%f ", pos[i][2]/maxZ*3.1415926);
			fprintf(fvel, "%d\n", grab);
		}
	}
	fclose(fvel);
}