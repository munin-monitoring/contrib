#!/usr/bin/env python
#coding:utf-8
import json
from munin import MuninPlugin
from analy_main_log import NGINX_MAIN_LOG_AVERAGE


class NormalRequestsPlugin(MuninPlugin):
    title = "xcf_main_log: detail of normal Request"
    args = "--base 1000 -l 0"
    vlabel = "Request per second"
    scaled = False
    category = "log"

    def set_data(self, data):
        self.data = data

    @property
    def fields(self):
        return [
           ("login_requests", dict(
                label = "Login_requests",
                info = "Login requests per second",
                min = "0",
            )),
            ("anonym_requests", dict(
                label = "Anonym_requests",
                info = "Anonym requests per second",
                min = "0",
            )),
            ("return_requests", dict(
                label = "Return_requests",
                info = "Cookie requests per second",
                min = "0",
            )),
            ("new_requests", dict(
                label = "New_requests",
                info = "nonCookie requests per second",
                min = "0",
            ))
                ]

    def execute(self):
        login_requests = self.data.get("login_requests")
        anonym_requests = self.data.get("anonym_requests")
        cookie_requests = self.data.get("cookie_requests")
        noncookie_requests = self.data.get("noncookie_requests")
        print "login_requests.value %s" %  login_requests
        print "anonym_requests.value %s" %  anonym_requests
        print "return_requests.value %s" % cookie_requests
        print "new_requests.value %s" % noncookie_requests


if __name__ == '__main__':
    with open(NGINX_MAIN_LOG_AVERAGE, 'r') as f:
        data_average_json = f.readline()
    data = json.loads(data_average_json)

    plugin = NormalRequestsPlugin()
    plugin.set_data(data)
    plugin.run()
