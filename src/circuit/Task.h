/*
 * Task.h
 *
 *  Created on: Aug 28, 2014
 *      Author: rlcevg
 */

#ifndef TASK_H_
#define TASK_H_

#include <memory>

namespace circuit {

class CTask {
public:
	template<typename _Callable, typename... _Args>
		explicit CTask(_Callable&& __f, _Args&&... __args) : onComplete(nullptr) {
			__b = _M_make_routine(std::__bind_simple(std::forward<_Callable>(__f), std::forward<_Args>(__args)...));
		}
	virtual ~CTask();

	void Run();
	void SetOnComplete(std::shared_ptr<CTask>);

private:
    struct _Impl_base;
    typedef std::shared_ptr<_Impl_base>	__shared_base_type;
	struct _Impl_base {
		__shared_base_type	_M_this_ptr;
		inline virtual ~_Impl_base();
		virtual void _M_run() = 0;
	};
	template<typename _Callable> struct _Impl : public _Impl_base {
		_Callable _M_func;
		_Impl(_Callable&& __f) : _M_func(std::forward<_Callable>(__f)) {}
		void _M_run() { _M_func(); }
	};

	template<typename _Callable> std::shared_ptr<_Impl<_Callable>> _M_make_routine(_Callable&& __f) {
		// Create and allocate full data structure, not base.
		return std::make_shared<_Impl<_Callable>>(std::forward<_Callable>(__f));
	}

	__shared_base_type __b;

	std::shared_ptr<CTask> onComplete;
};

inline CTask::_Impl_base::~_Impl_base() = default;

} // namespace circuit

#endif // TASK_H_