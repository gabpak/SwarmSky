#ifndef INC_ISR_HAL_H_
#define INC_ISR_HAL_H_

#include "main.h"
#include <vector>
#include <algorithm>

template<class T>
class ISR{
public:
	ISR(){
		ISR_LIST.clear();
	}
	~ISR(){
		ISR_LIST.clear();
	}

	void add(T* obj){
		ISR_LIST.push_back(obj);
	}

	void remove(T* obj){
		if(ISR_LIST.size() == 0){
			return;
		}
		ISR_LIST.erase(std::find(ISR_LIST.begin(), ISR_LIST.end(), obj));
	}

	T* get(uint16_t index){
	    if(index >= ISR_LIST.size() || ISR_LIST[index] == nullptr){
	        return nullptr;
	    }
	    return ISR_LIST[index];
	}

	uint16_t size(){
		return ISR_LIST.size();
	}

private:
	std::vector<T*> ISR_LIST;
};


#endif /* INC_ISR_HAL_H_ */
