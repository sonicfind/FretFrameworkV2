#pragma once
#include <queue>
#include <mutex>
#include <functional>

class TaskQueue
{
	std::jthread m_threads[64];
	size_t m_threadCount = 0;
	size_t m_numActiveThreads;

	std::queue<std::function<void()>> m_queue;
	std::mutex m_mutex;
	
	std::condition_variable m_mainCondition;
	std::condition_variable m_threadCondition;

	void add(const std::function<void()>& func);
	void run(const std::function<void()>& func);

	void(TaskQueue::* m_taskFunction)(const std::function<void()>&) = nullptr;

public:
	static TaskQueue& getInstance();

	void startThreads(size_t threadCount);
	void stopThreads();
	void addTask(const std::function<void()>& func);
	bool peekCompletedTasks();
	void waitForCompletedTasks();

private:
	static TaskQueue s_queue;
	TaskQueue();
	~TaskQueue();
};
