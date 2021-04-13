#ifndef __OBSERVER_HPP__
#define __OBSERVER_HPP__


/*
 * C++ Design Patterns: Observer
 * Author: Jakub Vojvoda [github.com/JakubVojvoda]
 * 2016
 *
 * Source code is licensed under MIT License
 * (for more details see LICENSE)
 *
 */

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

using namespace std;

template <typename StateT>
class Subject;

/*
 * Observer
 * defines an updating interface for objects that should be notified
 * of changes in a subject
 */
template <typename StateT>
class Observer
{
public:
	virtual ~Observer() {}

	virtual StateT get_state() = 0;
	virtual void update( Subject<StateT> *subject ) = 0;
};

/*
 * Concrete Observer
 * stores state of interest to ConcreteObserver objects and
 * sends a notification to its observers when its state changes

class ConcreteObserver : public Observer
{
public:
        ConcreteObserver( const int state ) :
                observer_state( state ) {}

        virtual ~ConcreteObserver() {}

        virtual int get_state() {
                return observer_state;
        }

        virtual void update( Subject *subject );

private:
        int observer_state;
};
*/

/*
 * Subject
 * knows its observers and provides an interface for attaching
 * and detaching observers
 */
template< typename StateT >
class Subject
{
public:
	virtual ~Subject() {}

	int attach( shared_ptr<Observer<StateT>> observer) {
		observers.push_back(observer);
		return observers.size() - 1;
	}

	void detach( const int index ) {
		observers.erase( observers.begin() + index );
	}

	void detach( shared_ptr<Observer<StateT>> observer ) {
                auto iterator = std::find_if( observers.begin(), observers.end(),
                                              [ &observer ]( shared_ptr<Observer<StateT>> p )
						      { return addressof(*p) == addressof(*observer); });
		observers.erase( iterator );
        }

	void notify() {
		for ( unsigned int i = 0; i < observers.size(); i++ ) {
			observers.at( i )->update( this );
		}
	}

	virtual StateT get_state() = 0;
	virtual void set_state( const StateT s ) = 0;

	std::vector<shared_ptr<Observer<StateT>>> observers;
};

/*
 * Concrete Subject
 * stores state that should stay consistent with the subject's

class ConcreteSubject : public Subject
{
public:
        ~ConcreteSubject() {}

        int get_state() {
                return subject_state;
        }

        void set_state( const int s ) {
                subject_state = s;
        }
        // ...

private:
        int subject_state;
        // ...
};
*/


/*
void ConcreteObserver::update( Subject *subject )
{
	observer_state = subject->get_state();
	std::cout << "Observer state updated." << std::endl;
}
*/

#endif // __OBSERVER_HPP__
