## Script (Python) "munin_db_activity.py"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=
##title=
##
"""
Fetches data about the ZODB for the munin plugin "zope_db_activity".
Needs the Manager proxy role to work.
Only answers requests from localhost directly to zopes port.
"""

req = context.REQUEST

if req['HTTP_X_FORWARDED_FOR'] or req['REMOTE_ADDR'] != '127.0.0.1':
    return "This script can only be called frm localhost"

sec = 60*5 # 5 min is munins update frequency

now = float(DateTime())
then = now-sec

request = dict(chart_start=then,
               chart_end=now)

maindb = context.restrictedTraverse('/Control_Panel/Database/main')
cd =  maindb.getActivityChartData(200, request)

print cd['total_load_count'],cd['total_store_count'],cd['total_connections']

return printed
