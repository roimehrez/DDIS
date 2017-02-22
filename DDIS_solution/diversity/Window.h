#pragma once
#include "Mat.h"
#include <Eigen/Core>

using namespace Eigen;

namespace dis
{
	template <typename T>
	class Window
	{
	protected:
		const Mat<T> data_;
		int windowSub2ind(int i, int j) const
		{
			int result = data_.sub2ind(i + topLeftY_, j + topLeftX_);
			return result;
		}

	public:
		const int width_, height_;
		const int topLeftX_, topLeftY_;

		Window(int w, int h, int topLeftX_, int topLeftY_, const Mat<T>& data_a) :
			data_(data_a), width_(w), height_(h), topLeftX_(topLeftX_), topLeftY_(topLeftY_)
		{
		}
		
		Window(int w, int h, int topLeftX_, int topLeftY_, const Eigen::Map<Eigen::MatrixXi> data_a) :
			data_(&data_a(0)), width_(w), height_(h), topLeftX_(topLeftX_), topLeftY_(topLeftY_)
		{
		}

		const T& operator()(int i, int j) const
		{
			const T& res = data_(i + topLeftY_, j + topLeftX_);
			return res;
		}

		/*T& operator()(int i, int j)
		{
			T& res = data_(i + topLeftY_, j + topLeftX_);
			return res;
		}*/

		Eigen::Block<const Map < const Matrix<T, Dynamic, Dynamic>>> ToEigenBlock()
		{
			const T* dataArray = &data_(0, 0);
			const Map < const Matrix<T, Dynamic, Dynamic>> tmp(dataArray, data_.height, data_.width);
			return tmp.block(topLeftY_, topLeftX_, height_, width_);
		}


		class iterator
		{
		private:
			const Window& w_;
			const int* pCurrentItem_;
			const int* pCurrentColEnd_;
			const int* pLastItem_;
			int i_;
			int j_;

			bool currentIsValid() const
			{
				return pCurrentItem_ <= pLastItem_;
			}

		public:
			iterator(const Window& w) : w_(w), pCurrentItem_(&w_(0, 0)), pCurrentColEnd_(&w_(w_.height_ - 1, 0)), pLastItem_(&w_(w_.height_ - 1, w_.width_ - 1)), i_(0), j_(0)
			{
			}

			void next()
			{
				if (pCurrentItem_ == pCurrentColEnd_)
				{
					pCurrentColEnd_ += w_.data_.height;
					pCurrentItem_ = pCurrentColEnd_ - w_.height_ + 1;
					i_ = 0;
					++j_;
				}
				else
				{
					++pCurrentItem_ + 1;
					++i_;
				}

				DEBUG_ASSERT(&w_(i_, j_) == pCurrentItem_);
			}

			const T* currentItem()
			{
				if (currentIsValid()) { return pCurrentItem_; }
				return nullptr;
			}

			int i() const
			{
				return i_;
			}
			
			int j() const
			{
				return j_;
			}
		};

		iterator getIterator() const
		{
			return iterator(*this);
		}
	};
}
