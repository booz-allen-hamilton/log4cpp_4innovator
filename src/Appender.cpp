/*
 * Appender.cpp
 *
 * Copyright 2000, LifeLine Networks BV (www.lifeline.nl). All rights reserved.
 * Copyright 2000, Bastiaan Bakker. All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include "PortabilityImpl.hh"
#include <log4cpp/Appender.hh>
#include <iostream>

namespace log4cpp {
	static int appenders_nifty_counter; // zero initialized at load time
	static char appenderMapStorage_buf[sizeof(Appender::AppenderMapStorage)]; // memory for the nifty-counter singleton object
	Appender::AppenderMapStorage &Appender::_appenderMapStorageInstance = reinterpret_cast<Appender::AppenderMapStorage&> (appenderMapStorage_buf);

	Appender::AppenderMapStorage::AppenderMapStorage()  { 
		_allAppenders = new AppenderMap(); 
		//std::cout << "Allocated appenders!" << std::endl;
	};
	Appender::AppenderMapStorage::~AppenderMapStorage() { 
		//std::cout << "DeAllocating appenders!" << std::endl;
		_deleteAllAppendersWOLock(); 
		delete _allAppenders; 
	};
	
	Appender::AppenderMapStorageInitializer::AppenderMapStorageInitializer() {
		 if (appenders_nifty_counter++ == 0) {
			 new (&_appenderMapStorageInstance) AppenderMapStorage(); // placement new
		 }
 	}
	Appender::AppenderMapStorageInitializer::~AppenderMapStorageInitializer() {
		if (--appenders_nifty_counter == 0) {
			(&_appenderMapStorageInstance)->~AppenderMapStorage ();
		}
	}

    /* assume _appenderMapMutex locked */
    Appender::AppenderMap& Appender::_getAllAppenders() {
		return *_appenderMapStorageInstance._allAppenders;
    }

    Appender* Appender::getAppender(const std::string& name) {
        threading::ScopedLock lock(_appenderMapStorageInstance._appenderMapMutex);
        AppenderMap& allAppenders = Appender::_getAllAppenders();
        AppenderMap::iterator i = allAppenders.find(name);
        return (allAppenders.end() == i) ? NULL : ((*i).second);
    }
    
    void Appender::_addAppender(Appender* appender) {
        //REQUIRE(_allAppenders.find(appender->getName()) == _getAllAppenders().end())
        threading::ScopedLock lock(_appenderMapStorageInstance._appenderMapMutex);
        _getAllAppenders()[appender->getName()] = appender;
    }

    void Appender::_removeAppender(Appender* appender) {
        threading::ScopedLock lock(_appenderMapStorageInstance._appenderMapMutex);
		//private called from destructor only, but may be triggered by client code in several treads
        _getAllAppenders().erase(appender->getName());
    }
    
    bool Appender::reopenAll() {
        threading::ScopedLock lock(_appenderMapStorageInstance._appenderMapMutex);
        bool result = true;
        AppenderMap& allAppenders = _getAllAppenders();
        for(AppenderMap::iterator i = allAppenders.begin(); i != allAppenders.end(); i++) {
            result = result && ((*i).second)->reopen();
        }
        
        return result;
    }
    
    void Appender::closeAll() {
        threading::ScopedLock lock(_appenderMapStorageInstance._appenderMapMutex);
        AppenderMap& allAppenders = _getAllAppenders();
        for(AppenderMap::iterator i = allAppenders.begin(); i != allAppenders.end(); i++) {
            ((*i).second)->close();
        }
    }
    
    void Appender::_deleteAllAppenders() {
        threading::ScopedLock lock(_appenderMapStorageInstance._appenderMapMutex);
		_deleteAllAppendersWOLock();
	}    

    void Appender::_deleteAllAppendersWOLock() {
    /* assume _appenderMapMutex locked */
        AppenderMap& allAppenders = _getAllAppenders();
        for(AppenderMap::iterator i = allAppenders.begin(); i != allAppenders.end(); ) {
            Appender *app = (*i).second;
            i++; // increment iterator before delete or iterator will be invalid.
            delete (app);
        }
    }    

    Appender::Appender(const std::string& name) :
        _name(name) {
        _addAppender(this);
    }
    
    Appender::~Appender() {
        _removeAppender(this);
    }
}
