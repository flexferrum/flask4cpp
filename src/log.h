//
// Created by flexf on 13.03.2020.
//

#ifndef FLASK4CPP_LOG_H
#define FLASK4CPP_LOG_H

#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/trivial.hpp>

namespace flask4cpp
{
using LoggerType = boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>;

#define TO_LOG(L, LV) BOOST_LOG_SEV((L), boost::log::trivial::LV)

#define LOG_TRACE() TO_LOG((*log()), trace)
#define LOG_DEBUG() TO_LOG((*log()), debug)
#define LOG_INFO() TO_LOG((*log()), info)
#define LOG_WARNING() TO_LOG((*log()), warning)
#define LOG_ERROR() TO_LOG((*log()), error)
#define LOG_FATAL() TO_LOG((*log()), fatal)
} // namespace flask4cpp

#endif // FLASK4CPP_LOG_H
