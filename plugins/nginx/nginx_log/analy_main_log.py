#!/usr/bin/env python
#coding:utf-8
# generate data for munin-plugins
# should add this to crontab

import datetime
import re
import json

# output filepath
NGINX_MAIN_LOG_TOTAL = '/tmp/nginx_main_log_total'
NGINX_MAIN_LOG_AVERAGE = '/tmp/nginx_main_log_average'


class Record(object):
    ''' Parse lines of log into Record object

    Log format: 113.128.119.42 31/Jul/2012:00:00:01 +0800 - c4Xw4mzZ "GET /recipe/21534/?rtype=other&rpage=15&_=1343664024343 HTTP/1.1" 200 1005 "http://www.xiachufang.com/recipe/21534/###" "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Trident/4.0; KB974488) QQBrowser/6.14.15138.201" 0.170

    How to use:
        record = Record(line)
        record.date
        record.time

    Record attributes:
        date: datetime.datetime object which represents '29/Mar/2012'
        date_str: 29/Mar/2012
        time: datetime.datetime object which represents '29/Mar/2012 14:21:29'
        time_str: 14:21:29
        timezone: +0800
        remote_address: 221.2255.168.62
        uid: ''
        cookie: c4Xw4mzZ
        request: Get /recipe/624/ HTTP/1.1
        request_method: Get
        request_url: /recipe/624
        status: 200
        bytes_sent: 1078
        referer: None
        user_agent: Mozilla/5.0 (X11; U; Linux i686; en-US) AppleWebKit/534.10 (KHTML, like Gecko) Chrome/8.0.552.215 Safari/534.10
        request_time: 0.01
    '''

    def __init__(self, line):
        self.line = line.strip()
        self._url = None
        self.matches = [match if match != '-' else ''
                        for match in self._parse()]

    def _parse(self):
        '''return match object'''
        # remote_address local_time uid cookie request status bytes referer
        # user_agent request_time
        regrex = re.compile(
            r'(.*?) (.*?) (.*?) (.*?) (.*?) "(.*?)" (.*?) (.*?) "(.*?)" "(.*?)" (.*)')
        matches = regrex.match(self.line)

        return matches.groups()

    @property
    def remote_address(self):
        return self.matches[0]

    @property
    def date(self):
        return datetime.datetime.strptime(self.date_str, '%d/%b/%Y')

    @property
    def date_str(self):
        date, _ = self.matches[1].split(':', 1)
        return date

    @property
    def time(self):
        return datetime.datetime.strptime('%s %s' %
                                          (self.date_str, self.time_str),
                                          '%d/%b/%Y %H:%M:%S')

    @property
    def time_str(self):
        _, time = self.matches[1].split(':', 1)
        return time

    @property
    def timezone(self):
        return self.matches[2]

    @property
    def uid(self):
        return self.matches[3]

    @property
    def cookie(self):
        return self.matches[4]

    @property
    def request(self):
        return self.matches[5]

    @property
    def request_method(self):
        req_method, _ = self.request.split(' ', 1)
        return req_method

    @property
    def request_url(self):
        if self._url:
            return self._url

        self._url, _ = self.request.split(' ', 1)[1].rsplit(' ', 1)
        return self._url

    @property
    def status(self):
        return self.matches[6]

    @property
    def bytes_sent(self):
        return int(self.matches[7])

    @property
    def referer(self):
        return self.matches[8]

    @property
    def user_agent(self):
        return self.matches[9]

    @property
    def request_time(self):
        return self.matches[10]


class LogAnalyser(object):
    ''' GET the record of the log in certain minutes from now (5 minutes),
        anlysis them, and save the result into redis for munin-plugins
        add this to crontab

    How to use:
        la = LogAnalyser(logfile=filepath, seconds=300)
        la.analysis_records()
    '''

    line_terminators = ('\r\n', '\n', '\r')
    read_size = 1024
    robot_agents = set([
        'bot',
        'spider',
        'google',
        'baidu',
        'yahoo'
    ])

    robot_agents_detail = set([
        'google',   # Googlebot, Mediapartners-Google
        'baidu',  # Baiduspider, baidu Transcoder
        'sosospider',  # Sosospider
        'youdaobot',   # YoudaoBot
        '360spider',   # 360Spider
        'yahoo',  # Yahoo! Slurp China
        'sogou',  # Sogou web spider
        'bingbot',  # bingbot
        'yendexbot',  # YandexBot
    ])

    def __init__(self, logfilename, seconds=300):
        self.file = file(logfilename, 'r')
        self.seconds = seconds
        self.load_latest_records()

    def set_analy_time(self):
        self.seek_end()
        line = self.readline_backward()
        r = Record(line)
        now = r.time
        self.time_begin = now - datetime.timedelta(seconds=self.seconds+1)
        self.time_end = now - datetime.timedelta(seconds=1)

    def seek_end(self):
        ''' Set the file cursor to end
        '''
        self.seek(0, 2)

    def seek(self, pos, whence=0):
        ''' Set the file cursor at the position
        '''
        self.file.seek(pos, whence)

    def seek_line_backward(self):
        ''' Searches backwards from the current file position for a line terminator
        '''
        pos = end_pos = self.file.tell()
        read_size = self.read_size
        if pos > read_size:
            pos -= read_size
        else:
            pos = 0
            read_size = end_pos

        self.seek(pos)
        read_str = self.file.read(read_size)
        bytes_read = len(read_str)

        # The last charachter is a line terminator, don't count this one
        if bytes_read and read_str[-1] in self.line_terminators:
            bytes_read -= 1
        if read_str[-2:] == '\r\n' and '\r\n' in self.line_terminators:
            bytes_read -= 1

        while bytes_read > 0:
            # Scan backward, counting the newlines in this bufferfull
            i = bytes_read - 1
            while i >= 0:
                if read_str[i] in self.line_terminators:
                    self.seek(pos + i + 1)
                    return self.file.tell()
                i -= 1

            if pos == 0 or pos - self.read_size < 0:
                # Not enought lines in the buffer, send the whole file
                self.seek(0)
                return 0
            pos -= self.read_size
            self.seek(pos)
            read_str = self.file.read(read_size)
            bytes_read = len(read_str)
        return 0

    def readline_backward(self):
        ''' Set the position backward a line, and return the string of that line
        '''
        this_pos = self.file.tell()
        last_pos = self.seek_line_backward()
        self.seek(last_pos)
        line = self.file.read(this_pos - last_pos)
        self.seek(last_pos)
        return line

    def load_latest_records(self):
        self.set_analy_time()
        self.records = []
        self.seek_end()
        while True:
            line = self.readline_backward()
            if not line:
                break
            r = Record(line)
            if r.time <= self.time_begin:
                break
            if r.time > self.time_begin and r.time <= self.time_end:
                self.records.append(r)

    def is_normal_agent(self, record):
        if any([bot in record.user_agent.lower() for bot in self.robot_agents]):
            return False
        else:
            return True

    def init_data_total(self):
        self.data_total = dict(
            total_requests=0.0,
            request_time=0.0,

            robot_requests=0.0,
            normal_requests=0.0,

            # in normal request, cookied or not
            cookie_requests=0.0,
            noncookie_requests=0.0,

            # in normal request, user login or not
            login_requests=0.0,
            anonym_requests=0.0,

            # search or category hitrate
            search_click=0.0,
            search_display=0.0,
            category_click=0.0,
            category_display=0.0,
        )

    def update_total(self, record):
        self.data_total['total_requests'] += 1

    def update_request_time(self, record):
        try:
            self.data_total['request_time'] += float(record.request_time)
        except Exception:
            pass

    def update_robots_or_normal(self, record):
        if self.is_normal_agent(record):
            self.data_total['normal_requests'] += 1
        else:
            self.data_total['robot_requests'] += 1

    def update_login_or_anonym(self, record):
        if record.uid:
            self.data_total['login_requests'] += 1
        else:
            self.data_total['anonym_requests'] += 1

    def update_cookie_or_noncookie(self, record):
        if record.cookie:
            self.data_total['cookie_requests'] += 1
        else:
            self.data_total['noncookie_requests'] += 1

    def update_search_or_category_hitrate(self, record):
        if '/search/' in record.request_url:
            self.data_total['search_display'] += 1
        if '/recipe/' in record.request_url and '/search/' in record.referer:
            self.data_total['search_click'] += 1
        if re.search(r'/category/(\d+)/', record.request_url):
            self.data_total['category_display'] += 1
        if '/recipe/' in record.request_url and re.search(r'/category/(\d+)/', record.referer):
            self.data_total['category_click'] += 1

    def calculate_data_average(self):
        self.data_average = {}
        for (key, value) in self.data_total.items():
            self.data_average[key] = value / self.seconds
        self.data_average['request_time'] = self.data_total['request_time'] / self.data_total['total_requests']
        self.data_average['search_hitrate'] = self.data_total['search_click'] / \
            self.data_total['search_display'] if self.data_total['search_display'] > 0 else 0
        self.data_average['category_hitrate'] = self.data_total['category_click'] / \
            self.data_total['category_display'] if self.data_total['category_display'] > 0 else 0

    def analysis_records(self):
        self.init_data_total()

        for record in self.records:
            # for all agent
            self.update_total(record)
            self.update_request_time(record)
            self.update_robots_or_normal(record)

            # only for normal agent
            if self.is_normal_agent(record):
                self.update_login_or_anonym(record)
                self.update_cookie_or_noncookie(record)
                self.update_search_or_category_hitrate(record)

        self.calculate_data_average()


def main():
    logfile = '/oyster_nginx_log/xcf_main.access.log'
    #logfile = '/home/tizac/test/xcf.log'
    la = LogAnalyser(logfile, 300)
    la.analysis_records()
    data_average_json = json.dumps(la.data_average)
    data_total_json = json.dumps(la.data_total)

    with open(NGINX_MAIN_LOG_TOTAL, 'w') as f:
        f.write(data_total_json)

    with open(NGINX_MAIN_LOG_AVERAGE, 'w') as f:
        f.write(data_average_json)

if __name__ == '__main__':
    main()
