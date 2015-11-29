#pragma once

template<class T> class Singleton
{
	public:

		static T& getInstance(void)
		{
			static T instance;
			return instance;
		}

	protected:

		virtual ~Singleton()
		{

		}

		Singleton()
		{

		}
};
