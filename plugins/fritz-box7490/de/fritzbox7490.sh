#!/bin/bash
# FritzBox.sh

### Quelle des Codekerns - Target of die skript core ############################
# https://github.com/Tscherno/Fritzbox.sh                                       #
# compatible with Fritz.box Firmware 6.50 and higher, Horst Schmid, 2016-01-05  #
# /usr/local/addons/cuxd/user/FritzBox.sh                                       # 
#################################################################################

#####################################################################################################################################################
# Der Vorbereitungs-Job muss per cron gestartet werden - Thie prerunning job must be started by cron                                                #
# /etc/cron.d/fritzbox                                                                                                                              #
# 3,8,13,18,23,28,33,38,43,48,53,58 * * * * root /<patch to the job>/fritzbox7490.sh  -p <login passwd> (-u >login user if required>] #okay         #
#                                                                                                                                                   #
#                                                                                                                             Jo Hartmann V17.0126  #
#####################################################################################################################################################

# Deklaration der Dateipfade  
# Declaration of file pathes
  CPWMD5="/etc/munin/plugins/pre2run/cpwmd5"
  FRITZLOGIN="/login_sid.lua"
  FRITZWEBCM="/cgi-bin/webcm"
  FRITZHOME="/home/home.lua"
  CURLFILE="/var/tmp/FritzBoxCurl.txt"
  LOGFILE="/var/log/fritzbox7490.log"
  PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

# Wie werden die Webseiten aufgerufen
# How to call web pages
  WEBCLIENT="curl -s"

# Deklaration sonstiger Variablen
# Declaration of other variables
  FritzBoxURL="192.168.0.111"
  Username=""
  Passwd=""
  Debug=false

# Verarbeitung der Aufruf-Parameter
# Processing the script parameters
  while getopts u:p:d OPT; do
    case $OPT in
       u)
         Username=$OPTARG
         ;;
       p)
         Passwd=$OPTARG
         ;;
       d)
         Debug=true
         LOGFILE="/dev/stdout"
         ;;
 
    esac
  done

# Anmelde-Routine, Quelle: https://github.com/Tscherno/Fritzbox.sh, Horst Schmid, 2016-01-05
# Login routine, source:   https://github.com/Tscherno/Fritzbox.sh, Horst Schmid, 2016-01-05
  LOGIN(){
  	# We need an SessionInfoChallenge SID from the FB. Combined with the PW an 
	# MD5-Checksum needs to be calculated and send back.
	# 1. Are we already logged in?
	htmlLoginPage=$($WEBCLIENT "$FritzBoxURL$FRITZLOGIN")
	SessionInfoChallenge=$(echo "$htmlLoginPage" | sed -n '/.*<Challenge>\([^<]*\)<.*/s//\1/p')
	SessionInfoSID=$(echo "$htmlLoginPage" | sed -n '/.*<SID>\([^<]*\)<.*/s//\1/p')
	if $Debug; then echo "****************** LOGIN: Challenge $SessionInfoChallenge" >> $LOGFILE; fi
	if [ "$SessionInfoSID" = "0000000000000000" ]; then
		if $Debug; then echo "****************** LOGIN: Keine gueltige SID - login aufbauen" >> $LOGFILE; fi
		CPSTR="$SessionInfoChallenge-$Passwd"  # Combine Challenge and Passwd 
		if $Debug; then echo "****************** LOGIN: CPSTR: $CPSTR -> MD5" >> $LOGFILE; fi
		MD5=`$CPWMD5 $CPSTR`  # here the MD5 checksum is calculated
		RESPONSE="$SessionInfoChallenge-$MD5" 
		if $Debug; then echo "****************** LOGIN: login senden und SID herausfischen, MD5: $MD5" >> $LOGFILE; fi
		GETDATA="?username=$Username&response=$RESPONSE"
		if $Debug; then echo "****************** LOGIN: $GETDATA" >> $LOGFILE; fi
		SID=$($WEBCLIENT "$FritzBoxURL$FRITZLOGIN$GETDATA" | sed -n '/.*<SID>\([^<]*\)<.*/s//\1/p')
		if $Debug; then echo "****************** Logged in with SID=$SID" >> $LOGFILE; fi
	else
		SID=$SessionInfoSID
		if $Debug; then echo "****************** LOGIN: Bereits erfolgreiche SID: $SID" >> $LOGFILE; fi
	fi
	if [ "$SID" = "0000000000000000" ]; then
		if $Debug; then echo "****************** LOGIN: ERROR - Konnte keine gueltige SID ermitteln" >> $LOGFILE; fi
	fi
  }

# Definition der Linienlänge für die Ausgabe (MEZ oder MESZ?)
# Definition of line length for output (CET or CEST?)
  MESZ=$(echo $(date +"%Z") | awk '{print length($0)-3}')
  line="#####################################################"
  if [ "$MESZ" = "1" ]; then line="${line}#";fi

# Die alte Log-Datei für Differenzbetrachtungen sichern
# Save the old log file for differential views
  if ! $Debug; then mv ${LOGFILE} ${LOGFILE}.1; fi

# Startmeldung zur Ausgabe
# Start message for output
  echo $line                                                      > $LOGFILE
  echo "Startzeit der Auswertung: $(date +"%d.%m.%Y %X %Z")   #" >> $LOGFILE
  echo $line                                                     >> $LOGFILE
  echo                                                           >> $LOGFILE

# Abrage der Hard-/Software-Eigenschaften, funktioniert ohne Anmeldung!
# Query the hardware / software properties, works without login! 
  jBoxInfo=$($WEBCLIENT "$FritzBoxURL/jason_boxinfo.xml")
  fbName=$(echo "$jBoxInfo" | grep 'j:Name' | sed -n 's,.*>\(.*\)</.*,\1,p')
  fbHardw=$(echo "$jBoxInfo" | grep 'j:HW' | sed -n 's,.*>\(.*\)</.*,\1,p')
  fbRev=$(echo "$jBoxInfo" | grep 'j:Revision' | sed -n 's,.*>\(.*\)</.*,\1,p')
  fbSN=$(echo "$jBoxInfo" | grep 'j:Serial' | sed -n 's,.*>\(.*\)</.*,\1,p')
  fbVersion1=$(echo "$jBoxInfo" | grep 'j:Version' | sed -n 's,.*>\(.*\)</.*,\1,p')
  fbVersion2=$(echo "$fbVersion1" | sed -n 's/\(.*\)[.]\(.*\)[.]\(.*\)/\2.\3/p')

# Login und Abruf der benötigten Seiten
# Login and retrieve the required pages 
  LOGIN
  _PAGE_DSL_STATS=$($WEBCLIENT "$FritzBoxURL/internet/dsl_stats_tab.lua?sid=$SID")
  _PAGE_INET_STAT=$($WEBCLIENT "$FritzBoxURL/internet/inetstat_monitor.lua?sid=$SID")
  _PAGE_INET_COUN=$($WEBCLIENT "$FritzBoxURL/internet/inetstat_counter.lua?sid=$SID")
  echo $_PAGE_DSL_STATS > /var/log/fritzbox7490.txt

# Verarbeitung der Daten der Seite Online-Monitor 
# Processing the data of the Online Monitor page 
  _AKT_IP=$(echo "$_PAGE_INET_STAT" | sed -n 's/.*<br>IP-Adresse: \(.*\)<\/span>.*/\1/p')
  _VERBAB=$(echo "$_PAGE_INET_STAT" | sed -n 's/.*<span class="limited">verbunden seit \(.*\)<\/span>,.*/\1/p')

# Verarbeitung der Daten der Seite Online-Zähler
# Processing the data of the page Online counter 
  _ONLINE_Zeit=$(echo "${_PAGE_INET_COUN}" | tr -d "\n" | awk 'match($0,/Heute<\/td>.*"time">([0-9,:]+)<\/td>.*Gestern/,arr){print arr[1]};')
  _VOLUMEN_T=$(echo "${_PAGE_INET_COUN}"   | tr -d "\n" | awk 'match($0,/Heute<\/td>.*"vol">([0-9]+)<\/td>.*"vol">.*"vol">.*Gestern/,arr){print arr[1]};')
  _VOLUMEN_U=$(echo "${_PAGE_INET_COUN}"   | tr -d "\n" | awk 'match($0,/Heute<\/td>.*"vol">.*"vol">([0-9]+)<\/td>.*"vol">.*Gestern/,arr){print arr[1]};')
  _VOLUMEN_D=$(echo "${_PAGE_INET_COUN}"   | tr -d "\n" | awk 'match($0,/Heute<\/td>.*"vol">.*"vol">.*"vol">([0-9]+)<\/td>.*Gestern/,arr){print arr[1]};')
  _ANZ_VERB=$(echo "${_PAGE_INET_COUN}"    | tr -d "\n" | awk 'match($0,/Heute<\/td>.*"conn">([0-9,:]+)<\/td>.*Gestern/,arr){print arr[1]};')

# Verarbeitung der Daten der Seite DSL-Informationen --> DSL
# Processing the data of the page DSL information -> DSL 
  _DATARATE_IN=$(echo "${_PAGE_DSL_STATS}"       | awk 'match($0,/Aktuelle Datenrate.*"c3">([0-9]+)<\/td>.*Nahtlose Ratenadaption/,arr){print arr[1]};')
  _DATARATE_OUT=$(echo "${_PAGE_DSL_STATS}"      | awk 'match($0,/Aktuelle Datenrate.*"c4">([0-9]+)<\/td>.*Nahtlose Ratenadaption/,arr){print arr[1]};')

  _LATENZ_IN=$(echo "${_PAGE_DSL_STATS}"         | tr -d "\n" | awk 'match($0,/Latenz.*"c3">([0-9]+) ms<\/td>/,arr){print arr[1]};')
  _LATENZ_OUT=$(echo "${_PAGE_DSL_STATS}"        | tr -d "\n" | awk 'match($0,/Latenz.*"c4">([0-9]+) ms<\/td>/,arr){print arr[1]};')

  _NOISEMARGIN_IN=$(echo "${_PAGE_DSL_STATS}"    | tr -d "\n" | awk 'match($0,/rabstandsmarge.*"c3">([0-9]+)<\/td>.*gertausch/,arr){print arr[1]};')
  _NOISEMARGIN_OUT=$(echo "${_PAGE_DSL_STATS}"   | tr -d "\n" | awk 'match($0,/rabstandsmarge.*"c4">([0-9]+)<\/td>.*gertausch/,arr){print arr[1]};')

  _DSLLINELOSS_IN=$(echo "${_PAGE_DSL_STATS}"    | tr -d "\n" | awk 'match($0,/Leitungsd.*"c3">([0-9]+)<\/td>.*Leistungsreduzierung/,arr){print arr[1]};')
  _DSLLINELOSS_OUT=$(echo "${_PAGE_DSL_STATS}"   | tr -d "\n" | awk 'match($0,/Leitungsd.*"c4">([0-9]+)<\/td>.*Leistungsreduzierung/,arr){print arr[1]};')

  _DSLESERROR_IN=$(echo "${_PAGE_DSL_STATS}"     | tr -d "\n" | awk 'match($0,/FRITZ!Box<\/td>.*"c2">([0-9]+)<\/td>.*Vermittlungsstelle/,arr){print arr[1]};')
  _DSLESERROR_OUT=$(echo "${_PAGE_DSL_STATS}"    | tr -d "\n" | awk 'match($0,/Vermittlungsstelle<\/td>.*"c2">([0-9]+)<\/td>.*table>/,arr){print arr[1]};')

  _DSLSESERROR_IN=$(echo "${_PAGE_DSL_STATS}"    | tr -d "\n" | awk 'match($0,/FRITZ!Box<\/td>.*"c3">([0-9]+)<\/td>.*Vermittlungsstelle/,arr){print arr[1]};')
  _DSLSESERROR_OUT=$(echo "${_PAGE_DSL_STATS}"   | tr -d "\n" | awk 'match($0,/Vermittlungsstelle<\/td>.*"c3">([0-9]+)<\/td>.*table>/,arr){print arr[1]};')

  _DSLCRCERROR01_IN=$(echo "${_PAGE_DSL_STATS}"  | tr -d "\n" | awk 'match($0,/FRITZ!Box<\/td>.*"c4">([0-9,.]+)<\/td>.*Vermittlungsstelle/,arr){print arr[1]};')
  _DSLCRCERROR01_OUT=$(echo "${_PAGE_DSL_STATS}" | tr -d "\n" | awk 'match($0,/Vermittlungsstelle<\/td>.*"c4">([0-9,.]+)<\/td>.*table>/,arr){print arr[1]};')

  _DSLCRCERROR15_IN=$(echo "${_PAGE_DSL_STATS}"  | tr -d "\n" | awk 'match($0,/FRITZ!Box<\/td>.*"c5">([0-9]+)<\/td>.*Vermittlungsstelle/,arr){print arr[1]};')
  _DSLCRCERROR15_OUT=$(echo "${_PAGE_DSL_STATS}" | tr -d "\n" | awk 'match($0,/Vermittlungsstelle<\/td>.*"c5">([0-9]+)<\/td>.*table>/,arr){print arr[1]};')

#Resyncs der letzten 24h auslesen

#_RESYNCS=$(echo "$_PAGE_STATS_GRAPH" | awk  'BEGIN {summe = 0}; match($0, /\[\"sar:status\/StatResync\"\] = \"(.*)\"/, arr){for(i = 1; i <= split(arr[1],splitted,",");i++){summe += int(splitted[i])}};END {print summe};')

# Schreiben der Ausgabe Datei
# Write the output file 
  echo "AVM Typenbezeichnung:        $fbName"                       >> $LOGFILE
  echo "Firmeware-Version:           $fbVersion1"                   >> $LOGFILE
  echo "FritzOS:                     $fbVersion2"                   >> $LOGFILE
  echo "Hardware-Version:            $fbHardw"                      >> $LOGFILE
  echo "Reversion:                   $fbRev"                        >> $LOGFILE
  echo "Seriennummer:                $fbSN"                         >> $LOGFILE


  echo "Aktuelle IP-Adresse:         $_AKT_IP"                      >> $LOGFILE
  echo "Verbunden seit:              $_VERBAB"                      >> $LOGFILE

  echo "Datenrate IN:                $_DATARATE_IN kbit/s"          >> $LOGFILE
  echo "Datenrate OUT:               $_DATARATE_OUT kbit/s"         >> $LOGFILE
  echo "Latenz Empfangsrichtung:     $_LATENZ_IN ms"                >> $LOGFILE
  echo "Latenz Senderichtung:        $_LATENZ_OUT ms"               >> $LOGFILE
  echo "Störabstandsmarge IN:        $_NOISEMARGIN_IN db"           >> $LOGFILE
  echo "Störabstandsmarge OUT:       $_NOISEMARGIN_OUT db"          >> $LOGFILE
  echo "Leitungsdämpfung IN:         $_DSLLINELOSS_IN db"           >> $LOGFILE
  echo "Leitungsdämpfung OUT:        $_DSLLINELOSS_OUT db"          >> $LOGFILE
  echo "Sek. m. Fehlern (ES) IN:     $_DSLESERROR_IN"               >> $LOGFILE
  echo "Sek. m. Fehlern (ES) OUT:    $_DSLESERROR_OUT"              >> $LOGFILE
  echo "Sek. m. v. Fehl. (SES) IN:   $_DSLSESERROR_IN"              >> $LOGFILE
  echo "Sek. m. v. Fehl. (SES) OUT:  $_DSLSESERROR_OUT"             >> $LOGFILE
  echo "CRC Fehler (je Min) IN:      $_DSLCRCERROR01_IN"            >> $LOGFILE
  echo "CRC Fehler (je Min) OUT:     $_DSLCRCERROR01_OUT"           >> $LOGFILE
  echo "CRC Fehler (15 Min) IN:      $_DSLCRCERROR15_IN"            >> $LOGFILE
  echo "CRC Fehler (15 Min) OUT:     $_DSLCRCERROR15_OUT"           >> $LOGFILE

  echo "Stunden Online:              $_ONLINE_Zeit"                 >> $LOGFILE
  echo "Datenvolumen (gesamt):       $_VOLUMEN_T MB"                >> $LOGFILE
  echo "Datenvolumen (upload):       $_VOLUMEN_U MB"                >> $LOGFILE
  echo "Datenvolumen (download):     $_VOLUMEN_D MB"                >> $LOGFILE
  echo "Anz. der Verbindungen:       $_ANZ_VERB"                    >> $LOGFILE
  echo                                                              >> $LOGFILE
  echo $line                                                        >> $LOGFILE
  echo "Stoppzeit der Auswertung: $(date +"%d.%m.%Y %X %Z")   #"    >> $LOGFILE
  echo $line                                                        >> $LOGFILE
  #echo ${_PAGE_INET_COUN}
