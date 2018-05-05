#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

struct task
{
	string name;
	int r, d, c;
};

struct task_exec
{
	string name;
	double start, end;
	double speed;

	bool operator<(const task_exec& temp) const
	{
		return start > temp.start;
	}
};

// A bubble sort I modify to order first by ready time, then by deadline, then computation time(if needed).
void sort_tasks(vector<task>& tasks)
{
	task temp;
	int n = tasks.size();

	for(int x = n; x >= 0; x--)
	{
		for(int y = 0; y < n-1; y++)
		{
			if(tasks.at(y).r > tasks.at(y+1).r)
			{
				temp = tasks.at(y);
				tasks.at(y) = tasks.at(y+1);
				tasks.at(y+1) = temp;
			}

			else if(tasks.at(y).r == tasks.at(y+1).r)
			{
				if(tasks.at(y).d > tasks.at(y+1).d)
				{
					temp = tasks.at(y);
					tasks.at(y) = tasks.at(y+1);
					tasks.at(y+1) = temp;
				}

				else if(tasks.at(y).d == tasks.at(y+1).d)
				{
					if(tasks.at(y).c > tasks.at(y+1).c)
					{
						temp = tasks.at(y);
						tasks.at(y) = tasks.at(y+1);
						tasks.at(y+1) = temp;
					}
				}
			}
		}
	}
}

void gantt(vector<string> names, vector<double> slices, vector<double> speeds, double runtime)
{
	string names_row, quantums_row = "|", times_row = "0", speeds_row;
	double start, end, last = 0, interval;
	double quantum;

	for(int x = 0; x < names.size(); x++)
	{

		stringstream ss_speeds, ss_times, ss_names, ss_quantums;
		start = slices.at(2*x);
		end = slices.at(2*x + 1);
		interval = end - start;

		// Multiplying by a large amount here so the chart looks decent
		// even with small time slices. But chart may also be very long...
		quantum = ceil(interval / runtime * 80);

		if(start != last)
		{
			quantums_row += " IDLE |";

			names_row += "     ";
			times_row += "     ";
			speeds_row += "     ";

			ss_times << fixed << setprecision(2) << slices.at(2*x);
		}


		ss_names << internal << setfill(' ') << setw(quantum*3);
		ss_quantums << internal << setfill('_') << setw(quantum*3);
		ss_times << internal << setfill(' ') << setw(quantum*3);
		ss_speeds << internal << setfill(' ') << setw(quantum*3);

		ss_quantums << "|";
		quantums_row += ss_quantums.str();

		ss_names << names.at(x);
		names_row += ss_names.str();

		ss_times << fixed << setprecision(2) << end;
		times_row += ss_times.str();

		ss_speeds << fixed << setprecision(2) << speeds.at(x);
		speeds_row += ss_speeds.str();

		last = end;
	}

	cout << "Task Names: " << names_row << endl
		 << "Quantums:   " << quantums_row << endl
		 << "Times:      " << times_row << endl
		 << "Speeds:     " << speeds_row << endl;
}

bool opt(priority_queue<task_exec>& sched, vector<task>& task_arr)
{
	int n = task_arr.size();
	double density, big_density = 0;
	double start, end;
	double block_start = 0;
	double block_time = 0;

	priority_queue<task_exec> copy;

	vector<task> target;
	vector<task_exec> overlap;



	for(int x = 0; x < n; x++)
	{
		for(int y = 0; y < n; y++)
		{
			start = task_arr.at(x).r;
			end   = task_arr.at(y).d;

			if(end <= start)
				continue;

			// new vectors each time to reset
			vector<task> temp_target;
			vector<task_exec> temp_overlap;
			density = 0;
			block_time = 0;
			copy = sched;

			while(!copy.empty())
			{
				task_exec it = copy.top();
				copy.pop();

				if(it.start > end)
					break;

				if(it.start >= start)
				{
					if(it.end <= end)
					{
						block_time += it.end - it.start;
						temp_overlap.push_back(it);
					}
					else
					{
						block_time += end - it.start;

						task_exec temp = it;
						temp.end = end;
						temp_overlap.push_back(temp);
						break;
					}
				}
				else if(it.end <= end && it.end >= start)
				{
					block_time += it.end - start;
					task_exec temp = it;
					temp.start = start;
					temp_overlap.push_back(temp);
				}
			}

			for(int z = x; z < n; z++)
			{
				if(task_arr.at(z).r >= start && task_arr.at(z).d <= end)
				{
					temp_target.push_back(task_arr.at(z));
					density += task_arr.at(z).c;
				}
			}

			if(end - start == block_time)
				continue;

			density /= (end - start - block_time);

			if(density > big_density)
			{
				big_density = density;
				block_start = start;

				target = temp_target;
				overlap = temp_overlap;
			}
		}
	}

	if(big_density > 1)
	{
		for(int x = 0; x < target.size(); x++)
		{
			cout << target.at(x).name;
			if(x < target.size()-1)
				cout << ", ";
		}

		cout << " cannot finish before deadlines!" << endl;
		return false;
	}

	sort_tasks(target);

	double current_time = block_start, temp_cur_time, req_time;

	int overlap_counter = 0;

	for(int x = 0; x < target.size(); x++)
	{
		temp_cur_time = current_time;

		if(target.at(x).r > current_time)
			current_time = target.at(x).r;

		req_time = target.at(x).c / big_density;

		while(overlap_counter < overlap.size() && current_time >= overlap.at(overlap_counter).start)
		{
			if(current_time < overlap.at(overlap_counter).end)
				current_time = overlap.at(overlap_counter).end;

			overlap_counter++;
		}

		while(req_time > 0)
		{
			task_exec temp;
			temp.speed = big_density;
			temp.name = target.at(x).name;
			temp.start = current_time;

			if(overlap_counter < overlap.size() && current_time + req_time > overlap.at(overlap_counter).start)
			{
				temp.end = overlap.at(overlap_counter).start;
				//cout << "TEMPEND " << temp.end << endl;

				sched.push(temp);

				if(target.at(x).r > temp_cur_time)
					overlap.push_back(temp);

				req_time -= temp.end - temp.start;
				current_time = overlap.at(overlap_counter).end;
				overlap_counter++;
			}
			else
			{
				temp.end = current_time + req_time;

				sched.push(temp);

				if(target.at(x).r > temp_cur_time)
					overlap.push_back(temp);

				current_time += req_time;
				req_time = 0;
			}
		}

		if(target.at(x).r > temp_cur_time)
		{
			current_time = temp_cur_time;
			overlap_counter = 0;
		}
	}

	for(int x = 0; x < target.size(); x++)
	{
		for(int y = 0; y < task_arr.size(); y++)
		{
			if(target.at(x).name == task_arr.at(y).name)
			{
				task_arr.erase(task_arr.begin() + y);
				break;
			}
		}
	}

	return true;
}

int main()
{
	fstream inFile("input.txt");

	priority_queue<task_exec> sched;
	vector<task> task_arr;
	string temp;

	getline(inFile, temp);
	int num_tasks = stoi(temp);

	for(int x = 0; x < num_tasks; x++)
	{
		getline(inFile, temp);
		task temp_task;
		string ele;
		int index;

		index = temp.find('(');
		ele = temp.substr(0, index);
		temp_task.name = ele;
		temp = temp.substr(index + 1);

		index = temp.find(',');
		ele = temp.substr(0, index);
		temp_task.r = stoi(ele);
		temp = temp.substr(index + 1);

		index = temp.find(',');
		ele = temp.substr(0, index);
		temp_task.d = stoi(ele);
		temp = temp.substr(index + 1);

		index = temp.find(')');
		ele = temp.substr(0, index);
		temp_task.c = stoi(ele);

		task_arr.push_back(temp_task);
	}

	sort_tasks(task_arr);

	while(!task_arr.empty())
	{
		sort_tasks(task_arr);
		if(!opt(sched, task_arr))
		{
			cout << "unschedulable task set" << endl;
			return -1;
		}
	}

	cout << "Schedule after scaling: " << endl;
	double runtime = 0;
	vector<string> names;
	vector<double> times, speeds;

	cout << setw(10) << "TaskName"
		 << setw(15) << "Start Time"
		 << setw(10) << "End Time"
		 << setw(10) << "Speed" << endl;

	while(!sched.empty())
	{
		task_exec temp = sched.top();
		cout << setw(10) << temp.name
			 << setw(15) << temp.start
			 << setw(10) << temp.end
			 << setw(10) << temp.speed << endl;

		runtime += temp.end - temp.start;

		times.push_back(temp.start);
		times.push_back(temp.end);
		names.push_back(temp.name);
		speeds.push_back(temp.speed);
		sched.pop();
	}
	cout << setfill('-') << setw(45) << "-" << endl << "Gantt Chart: " << endl;
	gantt(names, times, speeds, runtime);

	return 0;
}
