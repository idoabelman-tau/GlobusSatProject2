
#ifndef TELEMETRYFILES_H_
#define TELEMETRYFILES_H_
//	---general

#define END_FILE_NAME_TX_REVC			"txr"
#define DIR_NAME_TX						"tx"
#define DIR_NAME_RX						"rx"
#define END_FILE_NAME_RX_REVC           "rxr"
#define DIR_NAME_RX_FRAME 				"rxf"
#define DIR_NAME_ANTENNA				"ant"
#define DIR_NAME_WOD_TLM				"wod"
#define	END_FILENAME_EPS_RAW_MB_TLM		"erm"
#define DIR_NAME_EPS_TLM				"eps"
#define END_FILENAME_EPS_RAW_CDB_TLM	"erc"
#define END_FILENAME_EPS_ENG_CDB_TLM	"eec"
#define	DIR_NAME_SOLAR_PANELS_TLM		"slr"
#define	DIR_NAME_LOGS					"log"



typedef enum {
// don't change the position of these
	tlm_eps = 0,
	tlm_tx,
	tlm_antenna,
	tlm_solar,
	tlm_wod,
// don't change the position of these


	tlm_tx_revc_NO_USE, // 5
	tlm_rx,
	tlm_rx_revc_NO_USE,
	tlm_rx_frame,
	tlm_eps_raw_mb_NOT_USED, // 9
	tlm_eps_eng_mb_NOT_USED,
	tlm_eps_raw_cdb_NOT_USED,
	tlm_eps_eng_cdb_NOT_USED,
	tlm_log //13
}tlm_type_t;
#endif /* TELEMETRYFILES_H_ */
