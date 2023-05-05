#include "TaskQueue.h"

TaskQueue TaskQueue::s_queue;

TaskQueue& TaskQueue::getInstance()
{
	return s_queue;
}

void TaskQueue::startThreads(size_t threadCount)
{
	if (m_threadCount != 0)
	{
		stopThreads();
		for (size_t i = 0; i < m_threadCount; ++i)
			m_threads[i].join();
	}

	m_threadCount = threadCount;

	if (m_threadCount == 0)
	{
		m_taskFunction = &TaskQueue::run;
		return;
	}

	m_taskFunction = &TaskQueue::add;
	if (m_threadCount >= 64)
		m_threadCount = 64;
	m_numActiveThreads = m_threadCount;

	for (size_t i = 0; i < m_threadCount; ++i)
		m_threads[i] = std::jthread(
			[&] (std::stop_token token) noexcept
			{
				auto pop = [&]() -> std::function<void()>
				{
					std::function<void()> func = nullptr;
					std::unique_lock lock(m_mutex);
					--m_numActiveThreads;
					if (m_numActiveThreads == 0)
						m_mainCondition.notify_one();

					m_threadCondition.wait(lock, [&] { return !m_queue.empty() || token.stop_requested(); });

					if (!token.stop_requested())
					{
						++m_numActiveThreads;
						func = m_queue.front();
						m_queue.pop();
					}
					return func;
				};

				while (std::function<void()> func = pop())
					func();
			});
}

void TaskQueue::stopThreads()
{
	std::scoped_lock lock(m_mutex);
	for (size_t i = 0; i < m_threadCount; ++i)
		m_threads[i].request_stop();
	m_threadCondition.notify_all();
}

void TaskQueue::waitForCompletedTasks()
{
	if (m_threadCount == 0)
		return;

	std::unique_lock lock(m_mutex);
	m_mainCondition.wait(lock, [&] { return peekCompletedTasks(); });
}

void TaskQueue::addTask(const std::function<void()>& func)
{
	(this->*m_taskFunction)(func);
}

bool TaskQueue::peekCompletedTasks()
{
	return m_queue.empty() && m_numActiveThreads == 0;
}

TaskQueue::TaskQueue()
{
	size_t threadCount = std::thread::hardware_concurrency();
	TaskQueue::startThreads(threadCount);
}

TaskQueue::~TaskQueue()
{
	TaskQueue::stopThreads();
}

void TaskQueue::add(const std::function<void()>& func)
{
	std::scoped_lock lock(m_mutex);
	m_queue.push(func);
	m_threadCondition.notify_one();
}

void TaskQueue::run(const std::function<void()>& func)
{
	func();
}
