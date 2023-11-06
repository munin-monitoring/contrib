import os
import time
import json
import urllib.parse
import urllib.request
from datetime import datetime, timedelta
import time

COMET_SERVER_URL = ""
COMET_ADMIN_USERNAME  = ""
COMET_ADMIN_PASSWORD  = ""

class CometServer(object):
	def __init__(self, url = COMET_SERVER_URL, adminuser = COMET_ADMIN_USERNAME, adminpass = COMET_ADMIN_PASSWORD):
		self.url = url
		self.adminuser = adminuser
		self.adminpass = adminpass
		self.apiCache = dict()

	def _get_response(self, endpoint, extraparams):
		apiRequest = urllib.request.Request(
			url = self.url + endpoint,
			data = urllib.parse.urlencode({
				"Username": self.adminuser,
				"AuthType": "Password",
				"Password": self.adminpass,
				**extraparams
			}).encode('utf-8')
		)
		return urllib.request.urlopen(apiRequest)
		
	def _request(self, endpoint, extraparams):
		"""Make API request to Comet Server and parse response JSON"""
		shouldCache = (len(extraparams) == 0)

		#runs if there are no extra parameters and the endpoint is in the dict
		if shouldCache and endpoint in self.apiCache:
			return self.apiCache[endpoint]

		ret = None
		with self._get_response(endpoint, extraparams) as apiResponse:
			ret = json.loads(apiResponse.read())
		
		#runs and caches the results if there are no parameters
		if shouldCache:
			self.apiCache[endpoint] = ret
		return ret

#---Online Devices
	def countOnline(self):
		#Count all online devices on the Comet Server
		version = self._request("api/v1/admin/meta/version", {})["Version"]
		count = 0
		for device in self._request("api/v1/admin/dispatcher/list-active", {}).values():
			if device["ReportedVersion"] == version:
				count += 1
		
		return count

	def countOutdated(self):
		#Count all online but outdated devices on the Comet Server
		version = self._request("api/v1/admin/meta/version", {})["Version"]
		count = 0
		for device in self._request("api/v1/admin/dispatcher/list-active", {}).values():
			if device["ReportedVersion"] != version:
				count += 1
		return count

	def countOffline(self):
		#Count all offline devices on the Comet Server
		count = 0
		for user in self._request("api/v1/admin/list-users-full", {}).values():
			count += len(user["Devices"])
		for device in self._request("api/v1/admin/dispatcher/list-active", {}).values():
			count -= 1
		return count


#---Server History
	def countUsers(self):
		#Count all usernames on the Comet Server
		return len(self._request("api/v1/admin/list-users", {}))

	def countBuckets(self):
		#Count all buckets on the Comet Server
		return len(self._request("api/v1/admin/storage/list-buckets", {}))

	def countDevices(self):
		#Count all devices on the Comet Server
		count = 0
		for user in self._request("api/v1/admin/list-users-full", {}).values():
			count += len(user["Devices"])
		return count

	def countBoosters(self):
		#Count all boosters on the Comet Server
		count = 0
		paidBoosters = {"engine1/exchangeedb", "engine1/mssql", "engine1/vsswriter", "engine1/hyperv", "engine1/windisk", "engine1/mongodb", "engine1/winmsofficemail"}
		for user in self._request("api/v1/admin/list-users-full", {}).values():
			singleCheck = set()
			hasItem = set()
			for source in user["Sources"].values():
				#This check is for the very small chance there is no Device tied to the Protected Item
				if source["OwnerDevice"] != "":
					hasItem.add(source["OwnerDevice"])
				if source["Engine"] in paidBoosters:
					singleCheck.add(source["OwnerDevice"]+source["Engine"])
			for deviceID in hasItem:
				device = user['Devices'][deviceID]
				if "PlatformVersion" in device and "distribution" in  device["PlatformVersion"] and device["PlatformVersion"]["distribution"] == "Synology DSM":
					singleCheck.add(deviceID+"Synology")
			count += len(singleCheck)
		return count

#---JobStatus
	def getJobsStatus(self):
		#Return job status from the last 24h on the Comet Server
		today = int(time.time())
		yesterday = int(time.time() - 86400)
		statusCount = {
			"Success": 0,
			"Error": 0,
			"Warning": 0,
			"Missed": 0,
			"Running": 0,
			"Skipped": 0
		}
		for job in self._request("api/v1/admin/get-jobs-for-date-range", {"Start": yesterday, "End": today}):
			if (5000 <= job["Status"] <= 5999):
				statusCount["Success"] += 1
			elif (6000 <= job["Status"] <= 6999):
				statusCount["Running"] += 1
			elif (job["Status"] == 7001):
				statusCount["Warning"] += 1
			elif (job["Status"] == 7004):
				statusCount["Missed"] += 1
			elif (job["Status"] == 7006):
				statusCount["Skipped"] += 1
			else:
				statusCount["Error"] += 1
		return statusCount

#---JobClassification
	def getJobsClassification(self):
		#Return job classification from the last 24h on the Comet Server
		today = int(time.time())
		yesterday = int(time.time() - 86400)
		classificationCount = {
			"Backup": 0,
			"Restore": 0,
			"Other": 0
		}
		for job in self._request("api/v1/admin/get-jobs-for-date-range", {"Start": yesterday, "End": today}):
			if (job["Classification"] == 4001):
				classificationCount["Backup"] += 1
			elif (job["Classification"] == 4002):
				classificationCount["Restore"] += 1
			else:
				classificationCount["Other"] += 1
		return classificationCount

#---Latency
	def getAPITime(self):
		#Return API time to the Comet Server
		start = datetime.now()
		_ = self._get_response("api/v1/admin/meta/server-config/get", {})
		end = datetime.now()
		return (end - start).total_seconds()*1000

#---OnlineCheck
	def checkOnline(self):
		#Checks if the server is online
		try:
			response = self._get_response("api/v1/admin/meta/server-config/get", {})
			return 1
		except:
			return 0