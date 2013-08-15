
////////////////////////////////////////////////////////////////////////////
//
// A class describing protocol contexts.
//
////////////////////////////////////////////////////////////////////////////

#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>
#include <list>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <sstream>

#include "socket.h"
#include "thread.h"

#include "pdu.h"
#include "address.h"
#include "flow.h"
#include "exception.h"
#include "reaper.h"

namespace analyser {

    // Context ID type.
    typedef unsigned long context_id;
    
    // Forward declarations.
    class context;
    class root_context;

    // Shared pointer types.
    typedef boost::shared_ptr<context> context_ptr;

    // Context class, describes the state around a 'flow' of data between
    // two endpoints at a particular network layer.
    class context : public reapable {
      private:

	// Next context ID to hand out.
	static context_id next_context_id;
	static unsigned long total_contexts;
	
	// This context's ID.
	context_id id;

      protected:

	// Watcher, tidies things up when they get old.
	watcher& w;

      public:

	// Default time-to-live.
	static const int default_ttl = 10;

	// The flow address.
	flow addr;

	// Lock for all context state.
	threads::mutex lock;

	// Use weak_ptr for the parent link, cause otherwise there's a
	// shared_ptr cycle.
	boost::weak_ptr<context> parent;

	// Child contexts.
	std::map<flow,context_ptr> children;

	// Constructor.
        context(watcher& w) : reapable(w), w(w) { 
	    id = next_context_id++; 
	    total_contexts++;
	    // parent is initialised to 'null'.
	}

	// Constructor, initialises parent pointer.
        context(watcher& w, context_ptr parent) : reapable(w), w(w) { 
	    id = next_context_id++; 
	    this->parent = parent;
	    total_contexts++;
	}

	// Given a flow address, returns the child context.
	context_ptr get_context(const flow& f) {

	    lock.lock();

	    context_ptr c;
	    
	    if (children.find(f) != children.end())
		c = children[f];

	    lock.unlock();
	    
	    return c;

	}

	// Adds a child context.
	void add_child(const flow& f, context_ptr c) {
	    lock.lock();
	    if (children.find(f) != children.end())
		throw exception("That context already exists.");
	    children[f] = c;
	    lock.unlock();
	}

	// Destructor.
	virtual ~context() { 
	    total_contexts--;
	    std::cerr << "Total contexts = " << total_contexts << std::endl;
	}

	// Returns constructor ID.
	// FIXME: Is it used?
	context_id get_id() { return id; }

	// Returns a context 'type'.
	virtual std::string get_type() = 0;

	// Delete myself.
	void reap() {

	    // Erase myself from my parent's child map.
	    // Should call my destructor, I guess.
	    context_ptr p = parent.lock();
	    if (p)
		p->children.erase(addr);

	}

    };

    // 'Root' context.  Root of a protocol stack, describes why that protocol
    // stack exists.  Basically reason for acquiring the data, indicated by
    // the LIID.
    class root_context : public context {
    private:

	// Don't set TTL on this, currently.  It's lifetime is managed by
	// the engine class.
	
	// LIID.
	std::string liid;

	// Address which caused acquisition of this data.
	address trigger_address;

      public:
	/* static context_ptr create(const std::string& liid) { */
	/*     root_context* c = new root_context(); */
	/*     c->liid = liid; */
	/*     return context_ptr(c); */
	/* } */
       root_context(watcher& w) : context(w) {
	    addr.src.layer = ROOT;
	    addr.dest.layer = ROOT;
	}
	void set_trigger_address(const tcpip::address& a) {
	    if (a.universe == a.ipv4) {
		if (a.addr.size() == 4)
		    trigger_address.assign(a.addr, NETWORK, IP4);
	    }
	    if (a.universe == a.ipv6) {
		if (a.addr.size() == 16)
		    trigger_address.assign(a.addr, NETWORK, IP6);
	    }
	}

	address& get_trigger_address() { return trigger_address; }

	std::string& get_liid() {
	    return liid;
	}

	void set_liid(const std::string& l) {
	    liid = l;
	}

	virtual std::string get_type() { return "root"; }
    };

};

#endif
