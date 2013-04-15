#!/usr/bin/env python
#coding:utf-8
import json
from munin import MuninPlugin
from analy_main_log import NGINX_MAIN_LOG_AVERAGE


class RequestsTimePlugin(MuninPlugin):
    title = "xcf_main_log: request time"
    args = "--base 1000 -l 0"
    vlabel = "Average server request time (seconds)"
    scaled = False
    category = "log"

    def set_data(self, data):
        self.data = data

    @property
    def fields(self):
        return [
           ("request_time", dict(
                label = "Requests_time",
                info = "Average server request time (seconds)",
                min = "0",
            ))
                ]

    def execute(self):
        request_time = self.data.get("request_time")
        print "request_time.value %s" % request_time


if __name__ == '__main__':
    with open(NGINX_MAIN_LOG_AVERAGE, 'r') as f:
        data_average_json = f.readline()
    data = json.loads(data_average_json)

    plugin = RequestsTimePlugin()
    plugin.set_data(data)
    plugin.run()
