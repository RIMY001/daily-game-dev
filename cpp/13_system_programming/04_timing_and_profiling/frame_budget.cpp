//	模拟300帧：每帧分三段计时，最后打印平均值与 p99
#include <Windows.h>
#include <cstdio>
#include <vector>
#include <algorithm>

static double NowMs() {
	static LARGE_INTEGER f{};
	if (!f.QuadPart) QueryPerformanceFrequency(&f);
	LARGE_INTEGER t; QueryPerformanceCounter(&t);
	return 1000.0 * double(t.QuadPart) / double(f.QuadPart);
}

int main() {
	constexpr int N = 300;
	std::vector<double> game, wait, total;
	for (int i = 0; i < N; ++i) {
		double t0 = NowMs();
		double cpu = 0.0;
		for (int j = 0; j < 20000; ++j) cpu += j * 0.001;	//	假装是 Tick 的逻辑负载
		double t1 = NowMs();
		game.push_back(t1 - t0);

		//	系统调度忙的时候，这个 Sleep 可能被拖到 2ms、5ms、甚至十几 ms。
		Sleep(1);	//	假装等一次事件/IO
		double t2 = NowMs();
		wait.push_back(t2 - t1);
		total.push_back(t2 - t0);
	}
	auto stat = [](std::vector<double>& v, const char* n) {
		std::sort(v.begin(), v.end());
		double sum = 0; for (double x : v) sum += x;
		printf("%-8s avg=%6.2f ms p50=%6.2f p99=%6.2f max=%6.2f\n",
			n, sum / v.size(), v[v.size() / 2], v[v.size() * 99 / 100], v.back());
	};
	stat(game, "logic"); stat(wait, "wait"); stat(total, "frame");
	return 0;
}