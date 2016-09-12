/* Copyright(C)
 * For free
 * All right reserved
 *
 */
#include <boost/thread.hpp>
#include <string>

using namespace std;

class RuleManager
{
    public:
        RuleManager(const std::string &config_file);
        ~RuleManager();
    private:
        /**
         * @brief _load_config_file load config files.
         *
         * @returns success:0 else error code(-1:read fail)
         */
        int _load_config_file();

        /**
         * @brief _start_task start task to load file with a time interveral.
         *
         * @param in interveral unit:s
         */
        void _start_task(const int interveral=5);

        /**
         * @brief _stop_task stop the task's load files.
         */
        void _stop_task();

        boost::shared_ptr<boost::thread> task_;
        std::string config_file_name_;
};
