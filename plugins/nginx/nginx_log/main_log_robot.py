#!/usr/bin/env python
#coding:utf-8
import json
from munin import MuninPlugin
from analy_main_log import NGINX_MAIN_LOG_AVERAGE


class RobotRequestsPlugin(MuninPlugin):
    title = "xcf_main_log: robot and normal Request"
    args = "--base 1000 -l 0"
    vlabel = "Request per second"
    scaled = False
    category = "log"

    def set_data(self, data):
        self.data = data

    @property
    def fields(self):
        return [
           ("robot_requests", dict(
                label = "Robot_requests",
                info = "Robot requests per second",
                min = "0",
            )),
            ("normal_requests", dict(
                label = "Normal_requests",
                info = "Normal requests per second",
                min = "0",
            ))
                ]

    def execute(self):
        robot_requests = self.data.get("robot_requests")
        normal_requests = self.data.get("normal_requests")
        print "robot_requests.value %s" % robot_requests
        print "normal_requests.value %s" % normal_requests


if __name__ == '__main__':
    with open(NGINX_MAIN_LOG_AVERAGE, 'r') as f:
        data_average_json = f.readline()
    data = json.loads(data_average_json)

    plugin = RobotRequestsPlugin()
    plugin.set_data(data)
    plugin.run()
