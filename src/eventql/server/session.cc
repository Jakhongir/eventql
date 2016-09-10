/**
 * Copyright (c) 2016 DeepCortex GmbH <legal@eventql.io>
 * Authors:
 *   - Paul Asmuth <paul@eventql.io>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License ("the license") as
 * published by the Free Software Foundation, either version 3 of the License,
 * or any later version.
 *
 * In accordance with Section 7(e) of the license, the licensing of the Program
 * under the license does not imply a trademark license. Therefore any rights,
 * title and interest in our trademarks remain entirely with us.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You can be released from the requirements of the license by purchasing a
 * commercial license. Buying such a license is mandatory as soon as you develop
 * commercial activities involving this program without disclosing the source
 * code of your own applications
 */
#include "eventql/server/session.h"
#include "eventql/util/wallclock.h"

namespace eventql {

Session::Session(
    const DatabaseContext* database_context) :
    database_context_(database_context),
    user_id_("<anonymous>"),
    heartbeat_last_(MonotonicClock::now()),
    heartbeat_interval_(kMicrosPerSecond / 10) {}

String Session::getUserID() const {
  std::unique_lock<std::mutex> lk(mutex_);
  return user_id_;
}

void Session::setUserID(const String& user_id) {
  user_id_ = user_id;
}

String Session::getEffectiveNamespace() const {
  std::unique_lock<std::mutex> lk(mutex_);
  return effective_namespace_;
}

void Session::setEffectiveNamespace(const String& ns) {
  std::unique_lock<std::mutex> lk(mutex_);
  effective_namespace_ = ns;
}

String Session::getDisplayNamespace() const {
  std::unique_lock<std::mutex> lk(mutex_);
  return display_namespace_;
}

void Session::setDisplayNamespace(const String& ns) {
  std::unique_lock<std::mutex> lk(mutex_);
  display_namespace_ = ns;
}

const DatabaseContext* Session::getDatabaseContext() {
  return database_context_;
}

void Session::setHeartbeatCallback(std::function<ReturnCode ()> cb) {
  heartbeat_cb_ = cb;
}

ReturnCode Session::triggerHeartbeat() {
  auto now = MonotonicClock::now();
  if (now >= heartbeat_last_ + heartbeat_interval_) {
    if (heartbeat_cb_) {
      return heartbeat_cb_();
    }

    heartbeat_last_ = now;
  }

  return ReturnCode::success();
}

} // namespace eventql
