## Script (Python) "munin_cache_parameters.py"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=
##title=
##
"""
Fetches data about the ZODB for the munin plugin "zope_cache_parameters".
Needs the Manager proxy role to work.
Only answers requests from localhost directly to zopes port.
"""

req = context.REQUEST

if req['HTTP_X_FORWARDED_FOR'] or req['REMOTE_ADDR'] != '127.0.0.1':
    return "This script can only be called frm localhost"

maindb = context.restrictedTraverse('/Control_Panel/Database/main')

print maindb.database_size(), # Total number of objects in the database
print maindb.cache_length(), # Total number of objects in memory from all caches
print len(maindb.cache_detail_length()) * maindb.cache_size() # Target number of objects in memory sum total

return printed
