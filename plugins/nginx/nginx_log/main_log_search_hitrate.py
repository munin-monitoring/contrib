#!/usr/bin/env python
#coding:utf-8
import json
from munin import MuninPlugin
from analy_main_log import NGINX_MAIN_LOG_AVERAGE


class HitratePlugin(MuninPlugin):
    title = "xcf_main_log: search or category hitrate (to explore a recipe)"
    args = "--base 1000 -l 0"
    vlabel = "Recipe hitrate"
    scaled = False
    category = "log"

    def set_data(self, data):
        self.data = data

    @property
    def fields(self):
        return [
           ("search_hitrate", dict(
                label = "Search_hitrate",
                info = "Recipe hitrate of search",
                min = "0",
            )),
            ("category_hitrate", dict(
                label = "Category_hitrate",
                info = "Recipe hitrate of category",
                min = "0",
            ))
                ]

    def execute(self):
        print "search_hitrate.value %s" % self.data.get("search_hitrate")
        print "category_hitrate.value %s" % self.data.get("category_hitrate")


if __name__ == '__main__':
    with open(NGINX_MAIN_LOG_AVERAGE, 'r') as f:
        data_average_json = f.readline()
    data = json.loads(data_average_json)

    plugin = HitratePlugin()
    plugin.set_data(data)
    plugin.run()
