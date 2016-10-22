########################################
# SOURCE USER CONFIGURATION (gprom_src/.gprom) and (~/.gprom}
if [ -f ${HOME}/${CONF_FILE} ]; then
	source ${HOME}/${CONF_FILE}
fi
if [ -f ${GPROM_CONF} ]; then
	source ${GPROM_CONF}
fi
########################################
# DETERMINE CONNECTION PARAMETER
if [ "${GPROM_IP}X" != "X" ]; then
	HOST="-host ${GPROM_IP}"
fi
if [ "${GPROM_DB}X" != "X" ]; then
	DB="-db ${GPROM_DB}"
fi
if [ "${GPROM_PASSWD}X" != "X" ]; then
	PASSWD="-passwd ${GPROM_PASSWD}"
fi
if [ "${GPROM_USER}X" != "X" ]; then
	USER="-user ${GPROM_USER}"
fi
if [ "${GPROM_PORT}X" != "X" ]; then
	PORT="-port ${GPROM_PORT}"
fi
CONNECTION_PARAMS="${HOST} ${DB} ${PORT} ${USER} ${PASSWD}"
########################################
# LOGGING
if [ "${GPROM_LOG}X" != "X" ]; then
	LOG="-log -loglevel ${GPROM_LOG}"
else
    LOG="-log -loglevel 0"
fi
########################################
# COMBINED CONFIGURATIONS USED BY SCRIPTS
GPROM_DL_PLUGINS="-Pparser dl -Panalyzer dl -Ptranslator dl"
