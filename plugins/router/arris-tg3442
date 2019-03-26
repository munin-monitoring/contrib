#!/usr/bin/env python3

"""
# MUNIN Plugin to monitor status of Arris TG3442 / TG2492LG-85
# and compatible cable modems
#
# Connect to the web-frontend and get current DOCSIS status of upstream and
# downstream channels. (Signal Power, SNR, Lock Status)
#
#
# Requirements:
#   - BeautifulSoup
#   - pycryptodome
#
# Configuration:
#   [arris]
#   env.url http://192.168.100.1
#   env.username admin
#   env.password yourpassword
#
# Parameters:
#   url      - URL to web-frontend
#   username - defaults to "admin"
#   password - valid password
#
#
# References: https://www.arris.com/products/touchstone-tg3442-cable-voice-gateway/
#
#
#
# Copyright (c) 2019 Daniel Hiepler <d-munin@coderdu.de>
#
# Permission to use, copy, and modify this software with or without fee
# is hereby granted, provided that this entire notice is included in
# all source code copies of any software which is or includes a copy or
# modification of this software.
#
# THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTY. IN PARTICULAR, NONE OF THE AUTHORS MAKES ANY
# REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE
# MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
# PURPOSE.
#
#
# Magic markers
#  #%# family=contrib
"""

import binascii
from bs4 import BeautifulSoup
from Crypto.Cipher import AES
import hashlib
import json
import re
import requests
import sys
import os


def login(session, url, username, password):
    """login to """
    # get login page
    r = session.get(f"{url}")
    # parse HTML
    h = BeautifulSoup(r.text, "lxml")
    # get session id from javascript in head
    current_session_id = re.search(r".*var currentSessionId = '(.+)';.*", h.head.text)[1]

    # encrypt password
    salt = os.urandom(8)
    iv = os.urandom(8)
    key = hashlib.pbkdf2_hmac(
        'sha256',
        bytes(password.encode("ascii")),
        salt,
        iterations=1000,
        dklen=128/8
    )
    secret = { "Password": password, "Nonce": current_session_id }
    plaintext = bytes(json.dumps(secret).encode("ascii"))
    associated_data = "loginPassword"
    # initialize cipher
    cipher = AES.new(key, AES.MODE_CCM, iv)
    # set associated data
    cipher.update(bytes(associated_data.encode("ascii")))
    # encrypt plaintext
    encrypt_data = cipher.encrypt(plaintext)
    # append digest
    encrypt_data += cipher.digest()
    # return
    login_data = {
        'EncryptData': binascii.hexlify(encrypt_data).decode("ascii"),
        'Name': username,
        'Salt': binascii.hexlify(salt).decode("ascii"),
        'Iv': binascii.hexlify(iv).decode("ascii"),
        'AuthData': associated_data
    }

    # login
    r = session.put(
        f"{url}/php/ajaxSet_Password.php",
        headers={
            "Content-Type": "application/json",
            "csrfNonce": "undefined"
        },
        data=json.dumps(login_data)
    )

    # parse result
    result = json.loads(r.text)
    # success?
    if result['p_status'] == "Fail":
        print("login failure", file=sys.stderr)
        exit(-1)
    # remember CSRF nonce
    csrf_nonce = result['nonce']

    # prepare headers
    session.headers.update({
        "X-Requested-With": "XMLHttpRequest",
        "csrfNonce": csrf_nonce,
        "Origin": f"{url}/",
        "Referer": f"{url}/"
    })
    # set credentials cookie
    session.cookies.set(
        "credential",
        "eyAidW5pcXVlIjoiMjgwb2FQU0xpRiIsICJmYW1pbHkiOiI4NTIiLCAibW9kZWxuYW1lIjoiV"
        "EcyNDkyTEctODUiLCAibmFtZSI6InRlY2huaWNpYW4iLCAidGVjaCI6dHJ1ZSwgIm1vY2EiOj"
        "AsICJ3aWZpIjo1LCAiY29uVHlwZSI6IldBTiIsICJnd1dhbiI6ImYiLCAiRGVmUGFzc3dkQ2h"
        "hbmdlZCI6IllFUyIgfQ=="
    )

    # set session
    r = session.post(f"{url}/php/ajaxSet_Session.php")

def docsis_status(session):
    """get current DOCSIS status page, parse and return channel data"""
    r = session.get(f"{url}/php/status_docsis_data.php")
    # extract json from javascript
    json_downstream_data = re.search(r".*json_dsData = (.+);.*", r.text)[1]
    json_upstream_data = re.search(r".*json_usData = (.+);.*", r.text)[1]
    # parse json
    downstream_data = json.loads(json_downstream_data)
    upstream_data = json.loads(json_upstream_data)
    # convert lock status to numeric values
    for d in [ upstream_data, downstream_data ]:
        for c in d:
            if c['LockStatus'] == "ACTIVE" or c['LockStatus'] == "Locked":
                c['LockStatus'] = 1
            else:
                c['LockStatus'] = 0
    return downstream_data, upstream_data


# -----------------------------------------------------------------------------
if __name__ == "__main__":
    # get config
    url = os.getenv("url")
    username = os.getenv("username")
    password = os.getenv("password")
    # validate config
    if not url or not username or not password:
        print("Set url, username and password first.", file=sys.stderr)
        exit(1)
    # create session
    session = requests.Session()
    # login with username and password
    login(session, url, username, password)
    # get DOCSIS status
    downstream, upstream = docsis_status(session)
    # prepare munin graph info
    graph_descriptions = [
        {
            "name": "up_signal",
            "title": "DOCSIS Upstream signal strength",
            "vlabel": "dBmV",
            "info": "DOCSIS upstream signal strength by channel",
            "data": upstream,
            "key": "PowerLevel"
        },
        {
            "name": "up_lock",
            "title": "DOCSIS Upstream lock",
            "vlabel": "locked",
            "info": "DOCSIS upstream channel lock status",
            "data": upstream,
            "key": "LockStatus"
        },
        {
            "name": "down_signal",
            "title": "DOCSIS Downstream signal strength",
            "vlabel": "dBmV",
            "info": "DOCSIS downstream signal strength by channel",
            "data": downstream,
            "key": "PowerLevel"
        },
        {
            "name": "down_lock",
            "title": "DOCSIS Downstream lock",
            "vlabel": "locked",
            "info": "DOCSIS downstream channel lock status",
            "data": downstream,
            "key": "LockStatus"
        },
        {
            "name": "down_snr",
            "title": "DOCSIS Downstream signal/noise ratio",
            "vlabel": "dB",
            "info": "SNR/MER",
            "data": downstream,
            "key": "SNRLevel"
        }
    ]

    # configure ?
    if len(sys.argv) > 1 and "config" in sys.argv[1]:
        # process all graphs
        for g in graph_descriptions:
            # graph config
            print(
                f"multigraph docsis_{g['name']}\n"
                f"graph_title {g['title']}\n" \
                f"graph_category network\n" \
                f"graph_vlabel {g['vlabel']}\n" \
                f"graph_info {g['info']}\n" \
                f"graph_scale no\n"
            )

            # channels
            for c in g['data']:
                # only use channels with PowerLevel
                if not c['PowerLevel']:
                    continue
                print(
                    f"channel_{c['ChannelID']}.label {c['ChannelID']} ({c['Frequency']} MHz)\n"
                    f"channel_{c['ChannelID']}.info Channel type: {c['ChannelType']}, Modulation: {c['Modulation']}"
                )

    # output values ?
    else:
        # process all graphs
        for g in graph_descriptions:
            print(f"multigraph docsis_{g['name']}")
            # channels
            for c in g['data']:
                # only use channels with PowerLevel
                if not c['PowerLevel']:
                    continue
                print(f"channel_{c['ChannelID']}.value {c[g['key']]}")
