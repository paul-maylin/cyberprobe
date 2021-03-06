
#include <cybermon/unrecognised.h>
#include <cybermon/manager.h>

using namespace cybermon;

void unrecognised::process_unrecognised_stream(manager& mgr, context_ptr c, 
					       pdu_iter s, pdu_iter e)
{

    std::vector<unsigned char> empty;
    address src, dest;
    src.set(empty, TRANSPORT, UNRECOGNISED);
    dest.set(empty, TRANSPORT, UNRECOGNISED);

    flow_address f(src, dest);

    unrecognised_stream_context::ptr fc = 
	unrecognised_stream_context::get_or_create(c, f);

    fc->lock.lock();

    try {
	mgr.unrecognised_stream(fc, s, e);
    } catch (std::exception& e) {
	fc->lock.unlock();
	throw;
    }

    fc->lock.unlock();

}


void unrecognised::process_unrecognised_datagram(manager& mgr, context_ptr c, 
						 pdu_iter s, pdu_iter e)
{

    std::vector<unsigned char> empty;
    address src, dest;
    src.set(empty, TRANSPORT, UNRECOGNISED);
    dest.set(empty, TRANSPORT, UNRECOGNISED);

    flow_address f(src, dest);

    unrecognised_datagram_context::ptr fc = 
	unrecognised_datagram_context::get_or_create(c, f);

    fc->lock.lock();

    try {
	mgr.unrecognised_datagram(fc, s, e);
    } catch (std::exception& e) {
	fc->lock.unlock();
	throw;
    }

    fc->lock.unlock();

}

