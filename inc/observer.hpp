#ifndef __OBSERVER_HPP__
#define __OBSERVER_HPP__

#include <iostream>
#include <vector>
#include <mutex>
#include <algorithm>
#include <memory>
#include "common.hpp"

using namespace std;
class Observer;

class Subject: std::enable_shared_from_this<Subject> {
	vector <std::shared_ptr<Observer>> views;
	int message;
	mutex views_mutex;

public:
	std::shared_ptr<Subject> getptr() {
		return shared_from_this();
	}

	void attach(std::shared_ptr<Observer> pobs) {
		lock_guard<mutex> lock(views_mutex);
		auto compare = [&pobs](vector<shared_ptr<Observer>>::reference item) {
				       return std::addressof(*item) ==
					       std::addressof(*pobs);
			       };
		unique_add_replace(views, pobs, compare);
	}

	void detach(std::shared_ptr<Observer> pobs) {
		lock_guard<mutex> lock(views_mutex);
		for (vector<shared_ptr<Observer>>::iterator iter = views.begin();
		     iter != views.end();
		     ++iter) {
			// iterator points to shared_ptr of observer,
			// use ** to get the observer object
			if ( std::addressof(**iter) ==
			     std::addressof(*pobs) ) {
				views.erase(iter);
				return;
			}
		}
	}
	void setMessage(int msg) {
		message = msg;
		notify();
	}
	int getMessage() {
		return message;
	}
	void notify();
};


class Observer: std::enable_shared_from_this<Observer>
{
	std::shared_ptr<Subject> model;
	int value;
public:
	Observer(std::shared_ptr<Subject> mod, int obs_val) {
		model = mod;
		value = obs_val;
		// Register me with the Subject, which is of type of shared ptr
		model->attach(shared_from_this());
	}

	std::shared_ptr<Observer> getptr() {
		return shared_from_this();
	}

	virtual void update() = 0;

protected:
	std::shared_ptr<Subject> getSubject() {
		return model;
	}
	int getValue() {
		return value;
	}
};


void Subject::notify() {
	// Publisher broadcasts
	lock_guard<mutex> lock(views_mutex);
	for (size_t i = 0; i < views.size(); i++)
		views[i]->update();
}

class DivObserver: public Observer {
public:
	DivObserver(std::shared_ptr<Subject> mod, int div): Observer(mod, div){}
	void update() {
		// "Pull" information of interest
		int v = getSubject()->getMessage();
		int d = getValue();
		cout << v << " div " << d << " is " << v / d << '\n';
	}
};

class ModObserver: public Observer {
public:
	ModObserver(std::shared_ptr<Subject> mod, int div): Observer(mod, div){}
	void update() {
		int v = getSubject()->getMessage();
		int d = getValue();
		cout << v << " mod " << d << " is " << v % d << '\n';
	}
};


#endif // __OBSERVER_HPP__
