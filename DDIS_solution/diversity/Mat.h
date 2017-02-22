#pragma once

namespace dis
{
	template <typename T>
	class Mat
	{
		T* innerArray;
	public:
		
		Mat(T* data, int w, int h) : 
			innerArray(data),
			width(w),
			height(h)
		{}

		const int width, height;

		int sub2ind(int i, int j) const
		{
			int result = i + j*height;
			return result;
		}

		const T& operator()(int i, int j) const
		{
			return innerArray[sub2ind(i, j)];
		}

		T& operator()(int i, int j)
		{
			return innerArray[sub2ind(i, j)];
		}
	};
}