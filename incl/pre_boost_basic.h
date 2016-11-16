
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/detail/xml_parser_writer_settings.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/aligned_storage.hpp>
#include <boost/noncopyable.hpp>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include <boost/filesystem.hpp>
//#include <boost/thread.hpp>
//#include <boost/thread/mutex.hpp>
