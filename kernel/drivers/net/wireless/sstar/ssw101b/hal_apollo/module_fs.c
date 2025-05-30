#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/kthread.h>
#include <linux/dcache.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <net/net_namespace.h>
#include "apollo.h"
#include "bh.h"
#include "hwio.h"
#include "wsm.h"
#include "sbus.h"
#include "debug.h"
#include "apollo_plat.h"
#include "sta.h"
#include "ap.h"
#include "scan.h"
#include "module_fs.h"
#include "svn_version.h"
#include "internal_cmd.h"
#ifdef CONFIG_SSTAR_MOULE_FS
#define PARAMAS_SET BIT(0)
#define PARAMAS_ON BIT(1)
static bool enable_echo = true;
static int num_echo = -1;

#define SSTAR_SHOW_MSG_MAX_SIZE PAGE_SIZE

#define SSTAR_CODE__EXAMPLE_CODE 0
#define SSTAR_CODE__MAX 1

extern struct Sstar_common *g_hw_priv;

#define Sstar_show_init(show, _buff)                                           \
	rcu_assign_pointer((show)->show_buff, _buff);                          \
	(show)->show_count = 0;                                                \
	(show)->show_size = SSTAR_SHOW_MSG_MAX_SIZE;                           \
	(show)->overflow = 0
#define Sstar_show_deinit(show)                                                \
	rcu_assign_pointer((show)->show_buff, NULL);                           \
	(show)->show_count = 0;                                                \
	(show)->show_size = 0;                                                 \
	(show)->overflow = 0
#define Sstar_module_show_put(_show, ...)                                              \
	do {                                                                           \
		int ret = 0;                                                           \
		char *_show_buff;                                                      \
		rcu_read_lock();                                                       \
		_show_buff = rcu_dereference((_show)->show_buff);                      \
		if ((_show_buff == NULL) || ((_show)->overflow == 1) ||                \
		    ((_show)->show_size <= (_show)->show_count)) {                     \
			if (_show_buff)                                                \
				Sstar_printk_debug(                                    \
					"overflow(%d),show_size(%d),show_count(%d)\n", \
					(_show)->overflow, (_show)->show_size,         \
					(_show)->show_count);                          \
			rcu_read_unlock();                                             \
			break;                                                         \
		}                                                                      \
		ret = snprintf(_show_buff + (_show)->show_count,                       \
			       (_show)->show_size - (_show)->show_count,               \
			       __VA_ARGS__);                                           \
		if (ret >= 0) {                                                        \
			(_show)->show_count += ret;                                    \
			if ((_show)->show_count >= (_show)->show_size) {               \
				(_show)->show_count = (_show)->show_size;              \
				(_show)->overflow = 1;                                 \
			}                                                              \
		} else {                                                               \
			(_show)->overflow = 1;                                         \
			Sstar_printk_always("buff overflow(%d)(%d)\n",                 \
					    (_show)->show_size,                        \
					    (_show)->show_count);                      \
		}                                                                      \
		rcu_read_unlock();                                                     \
	} while (0)

#define Sstar_module_put(_show, _src, _len)                                    \
	do {                                                                   \
		char *_show_buff;                                              \
		if ((_show)->show_count + _len > (_show)->show_size)           \
			(_show)->overflow = 1;                                 \
		if ((_show)->overflow)                                         \
			break;                                                 \
		rcu_read_lock();                                               \
		_show_buff = rcu_dereference((_show)->show_buff);              \
		if (_show_buff == NULL) {                                      \
			rcu_read_unlock();                                     \
			break;                                                 \
		}                                                              \
		memcpy(_show_buff + (_show)->show_count, _src, _len);          \
		(_show)->show_count += _len;                                   \
		rcu_read_unlock();                                             \
	} while (0)

struct mutex Sstar_module_fs_lock;
#define Sstar_modulefs_lock_int() mutex_init(&Sstar_module_fs_lock)
#define Sstar_modulefs_lock() mutex_lock(&Sstar_module_fs_lock)
#define Sstar_modulefs_unlock() mutex_unlock(&Sstar_module_fs_lock)
#define Sstar_modulefs_trylock() mutex_trylock(&Sstar_module_fs_lock)
#define Sstar_modulefs_lock_release() mutex_destroy(&Sstar_module_fs_lock)
#define Sstar_modulefs_lock_check() lockdep_assert_held(&Sstar_module_fs_lock)

struct Sstar_module_show {
	char __rcu *show_buff;
	int show_count;
	int show_size;
	int overflow;
};

struct Sstar_sting_s {
	const char *string;
	const char *string_pos;
	struct hlist_node hnode;
	bool found;
	unsigned int len;
	unsigned int params;
	bool (*string_func)(const char *pos_l, const char *pos_r, void *priv);
	void *priv;
};
#define SSTAR_STRING_MAX_LEN 32
#define SSTAR_STRING_HASHBITS 4
#define SSTAR_STRING_HASHENTRIES (1 << SSTAR_STRING_HASHBITS)

#define SSTAR_STRING_TABLE_INIT(_string, _string_func, _priv)                  \
	{                                                                      \
		.string = _string, .string_pos = NULL, .found = 0, .len = 0,   \
		.params = 0, .string_func = _string_func, .priv = _priv,       \
	}
/*
*
*Sstar log level control fs
*
*/
u32 Sstar_printk_mask = SSTAR_PRINTK_DEFAULT_MASK;
module_param(Sstar_printk_mask, int, 0644);

struct Sstar_printk_mask_table_s {
	const char *mask_string;
	const char *mask_info;
	u32 mask_val;
	struct hlist_node string_hnode;
};
#define SSTAR_PRINTK_MAX_STRING_LEN 32
#define SSTAR_PRINTK_HASHBITS 4
#define SSTAR_PRINTK_HASHENTRIES (1 << SSTAR_PRINTK_HASHBITS)
#define LOG_ON "ON"
#define LOG_OFF "OFF"
#define LOG_ALL "OPEN_ALL"
#define LOG_DEFAULT "DEFAULT"
#define LOG_CLEAR "CLEAR"
#define SSTAR_BRA_L '['
#define SSTAR_BRA_R ']'
#define SSTAR_ANG_L '<'
#define SSTAR_ANG_R '>'

#define SSTAR_PARAML SSTAR_BRA_L
#define SSTAR_PARAMR SSTAR_BRA_R
#define SSTAR_PARAML_S SSTAR_ANG_L
#define SSTAR_PARAMR_S SSTAR_ANG_R

#define Sstar_string_invalid(_header)                                          \
	(((_header) != SSTAR_SPACE) && ((_header) != SSTAR_LINEF) &&           \
	 ((_header) != SSTAR_ENTER) && ((_header) != SSTAR_TAIL))
#define OPTION_TABLE_INIT(string, val)                                         \
	{                                                                      \
		.opton_string = string, .option_val = val,                     \
	}
#define LOG_TABLE_INIT(string, info, val)                                      \
	{                                                                      \
		.mask_string = string, .mask_info = info, .mask_val = val,     \
	}
struct Sstar_option_table_s {
	const char *opton_string;
	u32 option_val;
};

struct hlist_head Sstar_printk_hash_head[SSTAR_PRINTK_HASHENTRIES];
static const struct Sstar_option_table_s option_table[] = {
	OPTION_TABLE_INIT(LOG_ON, 1),
	OPTION_TABLE_INIT(LOG_OFF, 0),
};
static struct Sstar_printk_mask_table_s printk_mask_table[] = {
	LOG_TABLE_INIT("LOG_ERR", "output err  level log",
		       SSTAR_PRINTK_MASK_ERR),
	LOG_TABLE_INIT("LOG_WARN", "output warn level log",
		       SSTAR_PRINTK_MASK_WARN),
	LOG_TABLE_INIT("LOG_INIT", "output init level log",
		       SSTAR_PRINTK_MASK_INIT),
	LOG_TABLE_INIT("LOG_EXIT", "output exit level log",
		       SSTAR_PRINTK_MASK_EXIT),
	LOG_TABLE_INIT("LOG_BUS", "output bus  level log",
		       SSTAR_PRINTK_MASK_BUS),
	LOG_TABLE_INIT("LOG_SCAN", "output scan level log",
		       SSTAR_PRINTK_MASK_SCAN),
	LOG_TABLE_INIT("LOG_P2P", "output p2p  level log",
		       SSTAR_PRINTK_MASK_P2P),
	LOG_TABLE_INIT("LOG_MGMT", "output mgmt level log",
		       SSTAR_PRINTK_MASK_MGMT),
	LOG_TABLE_INIT("LOG_LMAC", "output lmac level log",
		       SSTAR_PRINTK_MASK_LMAC),
	LOG_TABLE_INIT("LOG_AGG", "output agg  level log",
		       SSTAR_PRINTK_MASK_AGG),
	LOG_TABLE_INIT("LOG_AP", "output ap   level log", SSTAR_PRINTK_MASK_AP),
	LOG_TABLE_INIT("LOG_STA", "output sta  level log",
		       SSTAR_PRINTK_MASK_STA),
	LOG_TABLE_INIT("LOG_SMT", "output smart config	level log",
		       SSTAR_PRINTK_MASK_SMARTCONFIG),
	LOG_TABLE_INIT("LOG_WEXT", "output wext level log",
		       SSTAR_PRINTK_MASK_WEXT),
	LOG_TABLE_INIT("LOG_TX", "output tx level log", SSTAR_PRINTK_MASK_TX),
	LOG_TABLE_INIT("LOG_RX", "output rx level log", SSTAR_PRINTK_MASK_RX),
	LOG_TABLE_INIT("LOG_PM", "output pm level log", SSTAR_PRINTK_MASK_PM),
	LOG_TABLE_INIT("LOG_PLATFORM", "output platform level log",
		       SSTAR_PRINTK_MASK_PLATFROM),
	LOG_TABLE_INIT("LOG_BH", "output bh level log", SSTAR_PRINTK_MASK_BH),
	LOG_TABLE_INIT("LOG_CFG80211", "output cfg80211 level log",
		       SSTAR_PRINTK_MASK_CFG80211),
	LOG_TABLE_INIT("LOG_DEBUG", "output cfg80211 level log",
		       SSTAR_PRINTK_MASK_DEBUG),
	LOG_TABLE_INIT("LOG_WSM", "output wsm level log",
		       SSTAR_PRINTK_MASK_WSM),
};
/*
*
*Sstar cmd fs control
*
*/
struct Sstar_store_cmd_code {
	const char *label;
	const char *cmd_info;
	void __rcu *cmd_private;
	bool (*code_cmd)(struct Sstar_common *hw_priv,
			 struct Sstar_store_cmd_code *cmd_store,
			 const char *buf, int len);
	ssize_t (*show_cmd)(struct Sstar_common *hw_priv,
			    struct Sstar_store_cmd_code *cmd_store, char *buf,
			    bool show);
	bool echo_enable;
	bool echo_ready;
	struct Sstar_module_show echo;
	struct hlist_node hnode;
	struct list_head lhead;
};
#define SSTAR_SCAN_HASHBITS SSTAR_COMMON_HASHBITS
#define SSTAR_SCAN_HASHENTRIES (1 << SSTAR_SCAN_HASHBITS)

struct Sstar_conf_ifname {
	u8 if_name[IFNAMSIZ];
	u8 size;
};

struct Sstar_conf_channels {
	u8 *channels;
	u8 n_channels;
};

struct Sstar_conf_ssids {
	struct cfg80211_ssid *ssids;
	u8 n_ssids;
};

struct Sstar_conf_macs {
	struct ieee80211_internal_mac *mac;
	u8 n_macs;
};

struct Sstar_conf_ies {
	u8 *ies;
	u8 ie_len;
	u8 Sstar_ie;
};
struct Sstar_conf_integer {
	int *integers;
	u8 n_integers;
};
struct Sstar_conf_string {
	char *strings;
	struct hlist_node hnode;
	bool set;
};
struct Sstar_config_string_hash {
	struct hlist_head hlist_head[SSTAR_COMMON_HASHENTRIES];
	bool empty;
};
struct Sstar_module_ap_conf {
	struct Sstar_conf_ifname ifname;
	struct Sstar_conf_channels channel;
	struct Sstar_conf_ssids ssid;
	struct Sstar_conf_macs bssid;
};
struct Sstar_module_set_mode_conf {
	struct Sstar_conf_ifname ifname;
	struct Sstar_conf_channels channel;
	enum ieee80211_internal_iftype iftype;
};
struct Sstar_module_halt_conf {
	struct Sstar_conf_ifname ifname;
};
struct Sstar_scan_params {
	struct Sstar_conf_channels channels;
	struct Sstar_conf_ssids ssids;
	struct Sstar_conf_macs macs;
	struct Sstar_conf_ies ies;
	bool positive;
};
struct Sstar_start_monitor_params {
	struct Sstar_conf_channels channel;
	u8 channeltype;
};

struct Sstar_stainfo_req_params {
	enum nl80211_iftype type;
	struct Sstar_conf_macs req_mac;
	u8 req_channel;
	u8 req_rssi;
	u8 req_rate;
	u8 req_txrxbytes;
	u8 req_ssid;
};

struct Sstar_wsm_req_parmas {
	u8 req_adaptive;
	u8 req_pwrdcxo;
	u8 req_pwr;
	int txpwr_dcxo[4];
	u8 dcxo_index;
	int txpwr;
};
struct Sstar_probe_request {
	struct Sstar_conf_ies ies;
	struct Sstar_conf_integer n_request;
};

struct Sstar_sta_listen {
	struct Sstar_conf_ifname ifname;
	struct Sstar_conf_channels channel;
};

struct Sstar_ap_set_beacon {
	struct Sstar_conf_ifname ifname;
	struct Sstar_conf_ies ies;
};

struct Sstar_special_freq {
	struct Sstar_conf_integer channel;
	struct Sstar_conf_integer freq;
};
struct Sstar_channel_auto_select {
	struct Sstar_conf_integer start_channel;
	struct Sstar_conf_integer end_channel;
	struct Sstar_conf_integer version;
	struct Sstar_conf_channels channels;
};

struct Sstar_echo_params {
	int num;
};
#define SSTAR_CMD_MAX_STRING_LEN 32
#define SSTAR_CMD_HASHBITS 4
#define SSTAR_CMD_HASHENTRIES (1 << SSTAR_CMD_HASHBITS)

#define SSTAR_CODE_STR__ATTR_EXAMPLE_INFO "show example"
#define SSTAR_CODE_STR__ATTR_HELP_INFO "show all cmd help info"
#define SSTAR_CODE_STR__ATTR_SCAN_INFO                                         \
	"sta scan: scan channel=[1][2][3] ssid=[ap1][ap2] private=[ie]or<ie> mac=[mac1][mac2]"
#define SSTAR_CODE_STR__ATTR_START_MONITOR_INFO                                \
	"start_monitor: start_monitor channel=[x]"
#define SSTAR_CODE_STR__ATTR_STOP_MONITOR_INFO "stop monitor:stop_monitor"
#define SSTAR_CODE_STR__ATTR_ENABLE_ECHO_INFO                                  \
	"enable echo parmas:enable_echo num_echo=[1]"
#define SSTAR_CODE_STR__ATTR_DISABLE_ECHO_INFO "disable echo:disable_echo"
#define SSTAR_CODE_STR__ATTR_AP_INFO_INFO                                      \
	"get ap info:ap_info req=[rssi][channel[toprate][txrxbytes][ssid]"
#define SSTAR_CODE_STR__ATTR_STA_INFO_INFO                                     \
	"get ap info:sta_info req=[rssi][channel[toprate][txrxbytes][ssid] mac=[mac1][mac2]"
#define SSTAR_CODE_STR__ATTR_WSM_CMD_INFRO                                     \
	"wsm cmd:wsm_cmd adaptive=[on or off] txpwr_dcxo=[M][H][L][dcxo] txpwr=[power]"
#define SSTAR_CODE_STR__ATTR_MONITOR_SEND_PROBER_INFO                          \
	"monitor/sta interface send probe request send_probe num=[number of probe request] private=[ie]"
#ifdef CONFIG_SSTAR_STA_LISTEN
#define SSTAR_CODE_STR__ATTR_STA_LISTEN_INFO                                   \
	"sta_listen: sta_listen ifname=[wlan0 or p2p0 or other] channel=[x]"
#endif
#define SSTAR_CODE_STR__ATTR_AP_BEACON_SPECIAL_IFFO                            \
	"set_beacon: ap mode set special beacon,set_beacon ifname=[wlan0 of p2p0] special=[some special ie]"

#define SSTAR_CODE_STR__ATTR_SET_FREQ_INFO                                     \
	"set_freq: set special freq,set_freq channel=[xx],freq=[2300~2600]"
#define SSTAR_CODE_STR__ATTR_CLEAR_FREQ_INFO                                   \
	"clear_freq: clear special freq,clear_freq channel=[xx]"
#define SSTAR_CODE_STR__ATTR_CHIP_VER_INFO                                     \
	"chip_ver: get the version of the current running chip(101B/101B2)"
#define SSTAR_CODE_STR__ATTR_CH_SELECT_INFO                                    \
	"auto channel select:channel_select start=[x] end=[y] version=[0 or 1]"
#define SSTAR_CODE_STR__ATTR_STA_SPECIAL_SCAN_INFO                             \
	"special_scan:channel=[1][2][3] ssid=[ap1] private=[ie] positive=[true or false]"
#define SSTAR_CODE_STR__ATTR_CHIP_VERSION_INFO                                 \
	"chip_version:get version of current running chip"
#define SSTAR_CODE_STR__ATTR_AP_CONF_INFO                                      \
	"set ap configs:ap_config ifname=[xxx] channel=[x] ssid=[ap] bssid=[xx:xx:xx:xx:xx:xx]"
#define SSTAR_CODE_STR__ATTR_SET_MODE_INFO                                     \
	"set if mode:set_mode ifname=[xxxx] mode=[managed/monitor] channel=[x]"
#define SSTAR_CODE_STR__ATTR_HALT_INFO "halt:halt ifname[xxxx]"

#define SSTAR_CODE_STR__ATTR_EXAMPLE "example"
#define SSTAR_CODE_STR__ATTR_HELP "help"
#define SSTAR_CODE_STR__ATTR_ENABLE_ECHO "enable_echo"
#define SSTAR_CODE_STR__ATTR_DISABLE_ECHO "disable_echo"
#define SSTAR_CODE_STR__ATTR_SCAN "scan"
#define SSTAR_CODE_STR__ATTR_START_MONITOR "start_monitor"
#define SSTAR_CODE_STR__ATTR_STOP_MONITOR "stop_monitor"
#define SSTAR_CODE_STR__ATTR_AP_INFO "ap_info"
#define SSTAR_CODE_STR__ATTR_STA_INFO "sta_info"
#define SSTAR_CODE_STR__ATTR_WSM_CMD "wsm_cmd"
#define SSTAR_CODE_STR__ATTR_MONITOR_SEND_PROBER "send_probe"
#ifdef CONFIG_SSTAR_STA_LISTEN
#define SSTAR_CODE_STR__ATTR_STA_LISTEN "sta_listen"
#endif
#define SSTAR_CODE_STR__ATTR_AP_BEACON_SPECIAL "set_beacon"
#define SSTAR_CODE_STR__ATTR_SET_FREQ "set_freq"
#define SSTAR_CODE_STR__ATTR_CLEAR_FREQ "clear_freq"
#define SSTAR_CODE_STR__ATTR_CHIP_VER "chip_ver"
#define SSTAR_CODE_STR__ATTR_CH_SELECT "channel_select"
#define SSTAR_CODE_STR__ATTR_STA_SPECIAL_SCAN "special_scan"
#define SSTAR_CODE_STR__ATTR_CHIP_VERSION "chip_version"
#define SSTAR_CODE_STR__ATTR_AP_CONF "ap_config"
#define SSTAR_CODE_STR__ATTR_SET_MODE "set_mode"
#define SSTAR_CODE_STR__ATTR_HALT "halt"

struct hlist_head Sstar_cmd_hash_head[SSTAR_CMD_HASHENTRIES];
struct list_head Sstar_cmd_echo_ready_list;
static char *backup_show = NULL;
static int backup_show_size = 0;
static int ready_list_num = 0;

#define SSTAR_CMD_INIT(cmd_code, info, _store_cmd, _show_cmd)                  \
	{                                                                      \
		.label = cmd_code, .cmd_info = info, .code_cmd = _store_cmd,   \
		.show_cmd = _show_cmd, .cmd_private = NULL,                    \
	}
static bool
Sstar_module_attr_echo_enable(struct Sstar_common *hw_priv,
			      struct Sstar_store_cmd_code *cmd_store,
			      const char *buf, int len);
static bool
Sstar_module_attr_echo_disable(struct Sstar_common *hw_priv,
			       struct Sstar_store_cmd_code *cmd_store,
			       const char *buf, int len);
static bool Sstar_module_attr_example(struct Sstar_common *hw_priv,
				      struct Sstar_store_cmd_code *cmd_store,
				      const char *buf, int len);
static bool Sstar_module_attr_help(struct Sstar_common *hw_priv,
				   struct Sstar_store_cmd_code *cmd_store,
				   const char *buf, int len);
static bool Sstar_module_attr_scan(struct Sstar_common *hw_priv,
				   struct Sstar_store_cmd_code *cmd_store,
				   const char *buf, int len);
static ssize_t
Sstar_module_attr_default_show(struct Sstar_common *hw_priv,
			       struct Sstar_store_cmd_code *cmd_code, char *buf,
			       bool show);
static bool
Sstar_module_attr_start_monitor(struct Sstar_common *hw_priv,
				struct Sstar_store_cmd_code *cmd_store,
				const char *buf, int len);
static ssize_t
Sstar_module_attr_start_monitor_show(struct Sstar_common *hw_priv,
				     struct Sstar_store_cmd_code *cmd_store,
				     char *buf, bool show);
static ssize_t
Sstar_module_attr_stop_monitor_show(struct Sstar_common *hw_priv,
				    struct Sstar_store_cmd_code *cmd_store,
				    char *buf, bool show);
static bool
Sstar_module_attr_stop_monitor(struct Sstar_common *hw_priv,
			       struct Sstar_store_cmd_code *cmd_store,
			       const char *buf, int len);
static bool Sstar_module_attr_ap_info(struct Sstar_common *hw_priv,
				      struct Sstar_store_cmd_code *cmd_store,
				      const char *buf, int len);
static bool Sstar_module_attr_sta_info(struct Sstar_common *hw_priv,
				       struct Sstar_store_cmd_code *cmd_store,
				       const char *buf, int len);
static bool Sstar_module_attr_wsm_cmd(struct Sstar_common *hw_priv,
				      struct Sstar_store_cmd_code *cmd_store,
				      const char *buf, int len);

static bool
Sstar_module_attr_send_probe_request(struct Sstar_common *hw_priv,
				     struct Sstar_store_cmd_code *cmd_store,
				     const char *buf, int len);
#ifdef CONFIG_SSTAR_STA_LISTEN
static bool Sstar_module_attr_sta_listen(struct Sstar_common *hw_priv,
					 struct Sstar_store_cmd_code *cmd_store,
					 const char *buf, int len);
#endif
static bool
Sstar_module_attr_ap_set_beacon(struct Sstar_common *hw_priv,
				struct Sstar_store_cmd_code *cmd_store,
				const char *buf, int len);
static bool
Sstar_module_attr_sta_special_scan(struct Sstar_common *hw_priv,
				   struct Sstar_store_cmd_code *cmd_store,
				   const char *buf, int len);

static bool Sstar_module_attr_set_freq(struct Sstar_common *hw_priv,
				       struct Sstar_store_cmd_code *cmd_store,
				       const char *buf, int len);
static bool Sstar_module_attr_clear_freq(struct Sstar_common *hw_priv,
					 struct Sstar_store_cmd_code *cmd_store,
					 const char *buf, int len);
static ssize_t
Sstar_module_attr_channel_select_show(struct Sstar_common *hw_priv,
				      struct Sstar_store_cmd_code *cmd_store,
				      char *buf, bool show);
static bool
Sstar_module_attr_channel_select(struct Sstar_common *hw_priv,
				 struct Sstar_store_cmd_code *cmd_store,
				 const char *buf, int len);
static bool
Sstar_module_attr_chip_version(struct Sstar_common *hw_priv,
			       struct Sstar_store_cmd_code *cmd_store,
			       const char *buf, int len);
#ifdef CONFIG_SSTAR_SUPPORT_AP_CONFIG
static bool
Sstar_module_attr_set_ap_config(struct Sstar_common *hw_priv,
				struct Sstar_store_cmd_code *cmd_store,
				const char *buf, int len);
#endif
static bool Sstar_module_attr_set_mode(struct Sstar_common *hw_priv,
				       struct Sstar_store_cmd_code *cmd_store,
				       const char *buf, int len);
static bool Sstar_module_attr_halt(struct Sstar_common *hw_priv,
				   struct Sstar_store_cmd_code *cmd_store,
				   const char *buf, int len);

struct Sstar_store_cmd_code cmd_code_buff[] = {
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_HELP,
		       SSTAR_CODE_STR__ATTR_HELP_INFO, Sstar_module_attr_help,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(
		SSTAR_CODE_STR__ATTR_EXAMPLE, SSTAR_CODE_STR__ATTR_EXAMPLE_INFO,
		Sstar_module_attr_example, Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_DISABLE_ECHO,
		       SSTAR_CODE_STR__ATTR_DISABLE_ECHO_INFO,
		       Sstar_module_attr_echo_disable,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_ENABLE_ECHO,
		       SSTAR_CODE_STR__ATTR_ENABLE_ECHO_INFO,
		       Sstar_module_attr_echo_enable,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_SCAN,
		       SSTAR_CODE_STR__ATTR_SCAN_INFO, Sstar_module_attr_scan,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_START_MONITOR,
		       SSTAR_CODE_STR__ATTR_START_MONITOR_INFO,
		       Sstar_module_attr_start_monitor,
		       Sstar_module_attr_start_monitor_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_STOP_MONITOR,
		       SSTAR_CODE_STR__ATTR_STOP_MONITOR_INFO,
		       Sstar_module_attr_stop_monitor,
		       Sstar_module_attr_stop_monitor_show),
	SSTAR_CMD_INIT(
		SSTAR_CODE_STR__ATTR_AP_INFO, SSTAR_CODE_STR__ATTR_AP_INFO_INFO,
		Sstar_module_attr_ap_info, Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_STA_INFO,
		       SSTAR_CODE_STR__ATTR_STA_INFO_INFO,
		       Sstar_module_attr_sta_info,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_WSM_CMD,
		       SSTAR_CODE_STR__ATTR_WSM_CMD_INFRO,
		       Sstar_module_attr_wsm_cmd,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_MONITOR_SEND_PROBER,
		       SSTAR_CODE_STR__ATTR_MONITOR_SEND_PROBER_INFO,
		       Sstar_module_attr_send_probe_request,
		       Sstar_module_attr_default_show),
#ifdef CONFIG_SSTAR_STA_LISTEN
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_STA_LISTEN,
		       SSTAR_CODE_STR__ATTR_STA_LISTEN_INFO,
		       Sstar_module_attr_sta_listen,
		       Sstar_module_attr_default_show),
#endif
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_AP_BEACON_SPECIAL,
		       SSTAR_CODE_STR__ATTR_AP_BEACON_SPECIAL_IFFO,
		       Sstar_module_attr_ap_set_beacon,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_STA_SPECIAL_SCAN,
		       SSTAR_CODE_STR__ATTR_STA_SPECIAL_SCAN_INFO,
		       Sstar_module_attr_sta_special_scan,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_SET_FREQ,
		       SSTAR_CODE_STR__ATTR_SET_FREQ_INFO,
		       Sstar_module_attr_set_freq,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_CLEAR_FREQ,
		       SSTAR_CODE_STR__ATTR_CLEAR_FREQ_INFO,
		       Sstar_module_attr_clear_freq,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_CH_SELECT,
		       SSTAR_CODE_STR__ATTR_CH_SELECT_INFO,
		       Sstar_module_attr_channel_select,
		       Sstar_module_attr_channel_select_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_CHIP_VERSION,
		       SSTAR_CODE_STR__ATTR_CHIP_VERSION_INFO,
		       Sstar_module_attr_chip_version,
		       Sstar_module_attr_default_show),
#ifdef CONFIG_SSTAR_SUPPORT_AP_CONFIG
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_AP_CONF,
		       SSTAR_CODE_STR__ATTR_AP_CONF_INFO,
		       Sstar_module_attr_set_ap_config,
		       Sstar_module_attr_default_show),
#endif
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_SET_MODE,
		       SSTAR_CODE_STR__ATTR_SET_MODE_INFO,
		       Sstar_module_attr_set_mode,
		       Sstar_module_attr_default_show),
	SSTAR_CMD_INIT(SSTAR_CODE_STR__ATTR_HALT,
		       SSTAR_CODE_STR__ATTR_HALT_INFO, Sstar_module_attr_halt,
		       Sstar_module_attr_default_show),
};

static unsigned int Sstar_hash_string_len(const char *pos, unsigned int pos_len)
{
	unsigned int i;
	unsigned int j;

	if (pos == NULL) {
		return 0;
	}
	for (i = 0; i < pos_len; i++) {
		if (pos[i] != SSTAR_SPACE)
			continue;
		for (j = i; j < pos_len; j++) {
			if (pos[j] != SSTAR_SPACE)
				return 0;
		}
		break;
	}

	return i;
}
static int Sstar_cmd_string_split(const char *pos, int len, const char **pos_l,
				  const char **pos_r)
{
	int i = 0;
	int j = 0;
	const char *l = NULL;
	const char *r = NULL;
	char next_pos;

	if (len == 0) {
		return -1;
	}
	Sstar_printk_debug("%s:%d %s\n", __func__, len, pos);
	for (i = 0; i < len; i++) {
		if ((pos[i] == SSTAR_PARAMR) || (pos[i] == SSTAR_PARAMR_S)) {
			Sstar_printk_debug("%s:left pos err\n", __func__);
			return -1;
		}

		if ((pos[i] != SSTAR_PARAML) && (pos[i] != SSTAR_PARAML_S)) {
			continue;
		}
		l = &pos[i];
		next_pos =
			pos[i] == SSTAR_PARAML ? SSTAR_PARAMR : SSTAR_PARAMR_S;
		for (j = i + 1; j < len; j++) {
			if ((pos[j] == SSTAR_PARAML) ||
			    (pos[j] == SSTAR_PARAML_S)) {
				Sstar_printk_debug("%s:right pos err\n",
						   __func__);
				return -1;
			}

			if (pos[j] != next_pos) {
				continue;
			}
			r = &pos[j];
			break;
		}
		break;
	}

	if ((l == NULL) && (r == NULL)) {
		return 0;
	} else if ((l == NULL) || (r == NULL)) {
		return -1;
	}

	*pos_l = l;
	*pos_r = r;

	return *l == SSTAR_PARAML ? 1 : 2;
}
static bool Sstar_string_parase_echo(const char *pos_l, const char *pos_r,
				     void *priv)
{
#define ECHO_ENABLE "enable"
#define ECHO_DISABLE "disable"
	struct Sstar_store_cmd_code *cmd_store =
		(struct Sstar_store_cmd_code *)priv;

	if (pos_r <= pos_l) {
		Sstar_printk_debug("%s:len err\n", __func__);
		return false;
	}

	if ((strlen(ECHO_ENABLE) == pos_r - pos_l) &&
	    (!memcmp(ECHO_ENABLE, pos_l, pos_r - pos_l))) {
		Sstar_printk_debug("%s:echo enanle\n", __func__);
		cmd_store->echo_enable = true;
	} else if ((strlen(ECHO_DISABLE) == pos_r - pos_l) &&
		   (!memcmp(ECHO_DISABLE, pos_l, pos_r - pos_l))) {
		Sstar_printk_debug("%s:echo disable\n", __func__);
		cmd_store->echo_enable = false;
	} else {
		Sstar_printk_debug("%s:unkown echo %s\n", __func__, pos_l);
		return false;
	}

	return true;
}
static bool Sstar_string_parase(struct Sstar_store_cmd_code *cmd_store,
				struct Sstar_sting_s *string_table,
				unsigned int table_size, const char *string,
				unsigned int string_len)
{
#define CMD_ECHO_STR "echo="
	unsigned int index = 0;
	const char *pos = string;
	const char *pos_end = string + string_len;
	const char *nospace_pos;
	const char *target_pos;
	struct hlist_head *hhead;
	struct hlist_node *node;
	struct hlist_head Sstar_string_hash_head[SSTAR_STRING_HASHENTRIES];
	struct Sstar_sting_s *string_node;
	struct Sstar_sting_s echo_str;
	unsigned int hash_index = 0;

	if (table_size < 0) {
		return false;
	}

	memset(&echo_str, 0, sizeof(struct Sstar_sting_s));

	echo_str.string = CMD_ECHO_STR;
	echo_str.priv = cmd_store;
	echo_str.string_func = Sstar_string_parase_echo;
	Sstar_common_hash_list_init(Sstar_string_hash_head,
				    SSTAR_STRING_HASHENTRIES);
	/*
	*first add echo string
	*/
	hash_index = Sstar_hash_index(echo_str.string, strlen(echo_str.string),
				      SSTAR_STRING_HASHBITS);
	hlist_add_head(&echo_str.hnode, &Sstar_string_hash_head[hash_index]);

	/*
	*second add requset string
	*/
	for (index = 0; index < table_size; index++) {
		unsigned int hash_index =
			Sstar_hash_index(string_table[index].string,
					 strlen(string_table[index].string),
					 SSTAR_STRING_HASHBITS);
		BUG_ON(strlen(string_table[index].string) >
		       SSTAR_STRING_MAX_LEN);

		hhead = &Sstar_string_hash_head[hash_index];

		hlist_for_each (node, hhead) {
			string_node =
				hlist_entry(node, struct Sstar_sting_s, hnode);
			if ((!strncmp(string_node->string,
				      string_table[index].string,
				      strlen(string_table[index].string))) &&
			    (strlen(string_table[index].string) ==
			     strlen(string_node->string))) {
				Sstar_printk_debug(
					"%s:string_node(%s) already in list\n",
					__func__, string_node->string);
				return false;
			}
		}
		hlist_add_head(&string_table[index].hnode,
			       &Sstar_string_hash_head[hash_index]);
		Sstar_printk_debug("%s:[%s]->[%d]\n", __func__,
				   string_table[index].string, hash_index);
	}

	Sstar_printk_debug("%s:len:%d tring:%s\n", __func__, string_len, pos);
	while (pos < pos_end) {
	next_str:
		/*
		*pos <= pos_end will return null,and break the loop
		*/
		nospace_pos = Sstar_skip_space(pos, pos_end - pos);

		if (nospace_pos == NULL) {
			Sstar_printk_debug("%s nospace_pos == NULL\n",
					   __func__);
			return pos == string ? false : true;
		}

		pos = nospace_pos;
		Sstar_printk_debug("%s:len:%zu nospace_pos:%s\n", __func__,
				   pos_end - pos, nospace_pos);
		if (pos[0] == SSTAR_TAIL)
			return pos == string ? false : true;
		else if ((pos[0] == SSTAR_LINEF) || (pos[0] == SSTAR_ENTER))
			return pos == string ? false : true;
		/*
		*find the postion of cmd end =
		*/
		target_pos = memchr(pos, SSTAR_EQUAL, pos_end - pos);
		if (target_pos == NULL) {
			Sstar_printk_always("%s:can not cmd =\n", __func__);
			return false;
		}
		Sstar_printk_debug("%s:len:%zu,str %s\n", __func__,
				   target_pos - pos + 1, pos);
		hhead = Sstar_hash_list(pos, target_pos - pos + 1,
					Sstar_string_hash_head,
					SSTAR_STRING_HASHBITS);

		if (hhead == NULL) {
			Sstar_printk_always("%s:hhead = NULL\n", __func__);
			return false;
		}
		hlist_for_each (node, hhead) {
			string_node =
				hlist_entry(node, struct Sstar_sting_s, hnode);
			Sstar_printk_debug(
				"%s:string_node(%s)(%zu),target:%zu:%s\n",
				__func__, string_node->string,
				strlen(string_node->string),
				target_pos - pos + 1, pos);
			if ((!strncmp(pos, string_node->string,
				      target_pos - pos + 1)) &&
			    (strlen(string_node->string) ==
			     target_pos - pos + 1)) {
				const char *tmp_pos = NULL;
				if (string_node->found == true) {
					Sstar_printk_debug(
						"%s:fund same string(%s)\n",
						__func__, string_node->string);
					return false;
				}
				tmp_pos = Sstar_skip_space(
					target_pos + 1,
					string_len - (target_pos + 1 - string));

				if (tmp_pos == NULL) {
					Sstar_printk_always(
						"%s:tmp_pos == NULL\n",
						__func__);
					return false;
				}
				string_node->found = true;
				string_node->string_pos = tmp_pos;
				string_node->len = pos_end - tmp_pos;

				if ((tmp_pos[0] != SSTAR_PARAML) &&
				    (tmp_pos[0] != SSTAR_PARAML_S)) {
					/*
					*find the postion of cmd end,use space between tow string
					*some cmd not need space ,so find enter or end of line
					*/
					pos = memchr(tmp_pos, SSTAR_SPACE,
						     pos_end - tmp_pos);
					if (pos == NULL) {
						pos = memchr(tmp_pos,
							     SSTAR_ENTER,
							     pos_end - tmp_pos);
						if (pos == NULL)
							pos = memchr(
								tmp_pos,
								SSTAR_LINEF,
								pos_end -
									tmp_pos);
					}
					if (string_node->string_func) {
						if (pos == NULL)
							pos = pos_end;
						if (string_node->string_func(
							    tmp_pos, pos,
							    string_node->priv) ==
						    false) {
							Sstar_printk_err(
								"%s:[%s] err tmp_pos (%p) pos (%p) %s\n",
								__func__,
								string_node
									->string,
								tmp_pos, pos,
								pos);
							return false;
						}
					}
					string_node->params = 1;
				} else {
					const char *pos_l;
					const char *pos_r;
					int res = 0;

					pos_r = tmp_pos;
					pos_l = tmp_pos;
					while (pos_r < pos_end) {
						res = Sstar_cmd_string_split(
							pos_l, pos_end - pos_l,
							&pos_l, &pos_r);

						if (res <= 0) {
							Sstar_printk_err(
								"%s not find l and r %s\n",
								__func__, pos);
							return false;
						}
						if ((pos_r - pos_l) == 1) {
							Sstar_printk_err(
								"%s param is null %s\n",
								__func__, pos);
							return false;
						}
						string_node->params++;
						if (string_node->string_func) {
							if (string_node->string_func(
								    pos_l + 1,
								    pos_r,
								    string_node
									    ->priv) ==
							    false) {
								Sstar_printk_err(
									"%s:[%s] err pos_l (%p) pos_r (%p) %s\n",
									__func__,
									string_node
										->string,
									pos_l + 1,
									pos_r,
									pos_l + 1);
								return false;
							}
						}
						pos_r++;
						pos_l = pos_r;

						if ((pos_l[0] !=
						     SSTAR_PARAML) &&
						    (pos_l[0] !=
						     SSTAR_PARAML_S)) {
							if (Sstar_string_invalid(
								    pos_l[0])) {
								Sstar_printk_err(
									"%s param format err %s\n",
									__func__,
									pos);
								return false;
							}
							break;
						}
					}

					pos = pos_r;
				}

				if (pos == NULL)
					pos = pos_end;
				else
					string_node->len = pos - tmp_pos;
				Sstar_printk_debug("[%s]:len %d,pos %s\n",
						   string_node->string,
						   string_node->len,
						   string_node->string_pos);
				goto next_str;
			}
		}

		Sstar_printk_always("%s:string is err %s\n", __func__, pos);

		return false;
	}

	return true;
}
static void *
Sstar_store_cmd_init_cmd_private(struct Sstar_store_cmd_code *cmd_store,
				 int len, gfp_t gfp)
{
	void *cmd_private;

	cmd_private = rcu_dereference(cmd_store->cmd_private);
	if (cmd_private) {
		if (cmd_store->show_cmd)
			cmd_store->show_cmd(Sstar_hw_priv_dereference(),
					    cmd_store, NULL, false);
		rcu_assign_pointer(cmd_store->cmd_private, NULL);
		synchronize_rcu();

		Sstar_kfree(cmd_private);
	}

	cmd_private = Sstar_kzalloc(len, gfp);

	return cmd_private;
}
static struct hlist_head *Sstar_cmd_hash_list(const char *string,
					      unsigned int len)
{
	return Sstar_hash_list(string, len, Sstar_cmd_hash_head,
			       SSTAR_CMD_HASHBITS);
}

static struct Sstar_store_cmd_code *
Sstar_store_cmd_find_cmd(const char *pos, unsigned int pos_len)
{
	unsigned char string_len = 0;
	struct hlist_head *hlist = NULL;
	struct Sstar_store_cmd_code *cmd_code;
	struct hlist_node *node;

	if (SSTAR_CMD_MAX_STRING_LEN < pos_len) {
		Sstar_printk_always("%s:[%s] pos_len is too long\n", __func__,
				    pos);
		return NULL;
	}

	string_len = Sstar_hash_string_len(pos, pos_len);

	if (string_len == 0) {
		Sstar_printk_always("%s:[%s] cmd len err\n", __func__, pos);
		return NULL;
	}

	hlist = Sstar_cmd_hash_list(pos, string_len);

	hlist_for_each (node, hlist) {
		cmd_code =
			hlist_entry(node, struct Sstar_store_cmd_code, hnode);
		if ((!strncmp(pos, cmd_code->label, string_len)) &&
		    (string_len == strlen(cmd_code->label))) {
			return cmd_code;
		}
	}
	Sstar_printk_always("%s:[%s] can not fund\n", __func__, pos);
	return NULL;
}
static bool Sstar_store_cmd_init_show(struct Sstar_module_show *show)
{
	char *show_buff;

	if ((show_buff = rcu_dereference(show->show_buff)) == NULL) {
		show_buff = Sstar_kzalloc(SSTAR_SHOW_MSG_MAX_SIZE, GFP_KERNEL);
	}

	if (show_buff == NULL) {
		return false;
	}
	Sstar_show_deinit(show);
	Sstar_show_init(show, show_buff);

	return true;
}

static void Sstar_store_cmd_deinit_show(struct Sstar_module_show *show)
{
	char *show_buff = rcu_dereference(show->show_buff);

	Sstar_show_deinit(show);
	if (show_buff) {
		synchronize_rcu();
		Sstar_kfree(show_buff);
		Sstar_printk_debug("%s:free(%p)\n", __func__, show_buff);
	}
}
static ssize_t
Sstar_module_attr_default_show(struct Sstar_common *hw_priv,
			       struct Sstar_store_cmd_code *cmd_code, char *buf,
			       bool show)
{
	struct Sstar_module_show echo_show;
	Sstar_show_init(&echo_show, buf);
	if (show == true)
		Sstar_module_show_put(&echo_show, cmd_code->echo.show_buff);
	return echo_show.show_count;
}
static bool Sstar_module_attr_example(struct Sstar_common *hw_priv,
				      struct Sstar_store_cmd_code *cmd_store,
				      const char *buf, int len)
{
	u8 index = 0;
	Sstar_module_show_put(&cmd_store->echo, "Sstar module cmd example\n");
	for (index = 0; index < 64; index++) {
		Sstar_module_show_put(&cmd_store->echo, "trace[%d]=[%d]\n",
				      index, hw_priv->multrx_trace[index]);
	}
	Sstar_module_show_put(&cmd_store->echo, "\n");
	Sstar_printk_always("%s\n", __func__);
	return true;
}
static bool Sstar_module_attr_help(struct Sstar_common *hw_priv,
				   struct Sstar_store_cmd_code *cmd_store,
				   const char *buf, int len)
{
	unsigned int index = 0;
	struct Sstar_module_show *show = &cmd_store->echo;

	Sstar_module_show_put(show, LIGHT "sstar Cmd Help-->:" NORMAL ENTER);
	for (index = 0; index < ARRAY_SIZE(cmd_code_buff); index++) {
		Sstar_module_show_put(show, LIGHT "[%s]" NORMAL ":%s\n",
				      cmd_code_buff[index].label,
				      cmd_code_buff[index].cmd_info);
	}
	Sstar_printk_always("%s\n", __func__);
	return true;
}

static struct ieee80211_sub_if_data *
Sstar_module_get_sdata_by_name(const char *name)
{
	struct net_device *net_dev = NULL;
	struct ieee80211_sub_if_data *sdata;

	ASSERT_RTNL();
	net_dev = __dev_get_by_name(&init_net, name);

	if ((net_dev == NULL) || (net_dev->ieee80211_ptr == NULL) ||
	    (net_dev->ieee80211_ptr->wiphy == NULL)) {
		return NULL;
	}

	if (net_dev->ieee80211_ptr->wiphy->privid != mac80211_wiphy_privid)
		return NULL;

	sdata = IEEE80211_DEV_TO_SUB_IF(net_dev);

	if (!strncmp(sdata->name, name, IFNAMSIZ))
		return sdata;

	Sstar_printk_err("%s: can not get(%s) dev\n", __func__, name);
	return NULL;
}
static bool Sstar_module_config_ifname(const char *posl, const char *posr,
				       void *priv)
{
	struct Sstar_conf_ifname *conf_ifname =
		(struct Sstar_conf_ifname *)priv;
	u8 len = posr - posl;

	Sstar_printk_debug("%s:posl(%p),posr(%p) %s\n", __func__, posl, posr,
			   posl);

	if (conf_ifname == NULL) {
		Sstar_printk_err("%s:priv NULL(%p),\n", __func__, priv);
		return false;
	}

	if (conf_ifname->size != 0) {
		Sstar_printk_err("%s:if name has been set\n", __func__);
		return false;
	}

	if (len > IFNAMSIZ) {
		Sstar_printk_err("%s:if name size too long\n", __func__);
		return false;
	}

	memcpy(conf_ifname->if_name, posl, len);
	conf_ifname->size = len;

	return true;
}
static bool Sstar_module_config_integers(const char *posl, const char *posr,
					 void *priv)
{
	struct Sstar_conf_integer *integers = (struct Sstar_conf_integer *)priv;

	int *new_integers = integers->integers;
	int *pos_integers = NULL;
	int interge_index = 0;

	if (priv == NULL) {
		Sstar_printk_err("%s:priv NULL(%p),\n", __func__, priv);
		return false;
	}

	if (integers->n_integers == 0xff) {
		Sstar_printk_err("%s:integers too mach(%d)\n", __func__,
				 integers->n_integers);
		return false;
	}

	if (new_integers && integers->n_integers) {
		new_integers =
			Sstar_krealloc(integers->integers,
				       (integers->n_integers + 1) * sizeof(int),
				       GFP_KERNEL);

		if (new_integers == NULL) {
			Sstar_printk_err("%s:integers alloc err(%d)\n",
					 __func__, integers->n_integers);
			return false;
		}
	} else {
		new_integers = Sstar_kzalloc(sizeof(int), GFP_KERNEL);

		if (new_integers == NULL) {
			Sstar_printk_err("%s:integers alloc err(%d)\n",
					 __func__, integers->n_integers);
			return false;
		}
	}

	integers->integers = new_integers;

	pos_integers = integers->integers + integers->n_integers;

	if (Sstar_accsii_to_int(posl, posr - posl, &interge_index) == false) {
		Sstar_printk_err("%s:interge_index (%d) err\n", __func__,
				 interge_index);
		return false;
	}

	integers->n_integers++;
	*pos_integers = interge_index;

	return true;
}
static bool Sstar_module_config_channels(const char *posl, const char *posr,
					 void *priv)
{
	struct Sstar_conf_channels *channels =
		(struct Sstar_conf_channels *)priv;

	u8 *new_channels = channels->channels;
	u8 *pos_channel;
	int channel_index = 0;

	Sstar_printk_debug("%s:posl(%p),posr(%p) %s\n", __func__, posl, posr,
			   posl);
	if (priv == NULL) {
		Sstar_printk_err("%s:priv NULL(%p),\n", __func__, priv);
		return false;
	}
	if (channels->n_channels >= SSTAR_MAX_SCAN_CHANNEL) {
		Sstar_printk_err("%s:channles too mach(%d)\n", __func__,
				 channels->n_channels);
		return false;
	}

	if ((posr - posl <= 0) || (posr - posl > 2)) {
		Sstar_printk_err("%s:channles len err\n", __func__);
		return false;
	}

	if (new_channels && channels->n_channels) {
		new_channels =
			Sstar_krealloc(channels->channels,
				       channels->n_channels + 1, GFP_KERNEL);
		if (new_channels == NULL) {
			Sstar_printk_err("%s:channles alloc err\n", __func__);
			return false;
		}

	} else {
		new_channels = Sstar_kzalloc(1, GFP_KERNEL);

		if (new_channels == NULL) {
			Sstar_printk_err("%s:channles kzalloc err\n", __func__);
			return false;
		}
	}
	channels->channels = new_channels;
	pos_channel = new_channels + channels->n_channels;

	if (Sstar_accsii_to_int(posl, posr - posl, &channel_index) == false) {
		Sstar_printk_err("%s:channel_index (%d) err\n", __func__,
				 channel_index);
		return false;
	}

	if (channel_index > 50) {
		Sstar_printk_err("%s:channel_index (%d) err\n", __func__,
				 channel_index);
		return false;
	}

	channels->n_channels++;
	*pos_channel = (u8)channel_index;
	return true;
}
static bool Sstar_module_config_ssids(const char *posl, const char *posr,
				      void *priv)
{
	struct Sstar_conf_ssids *ssids = (struct Sstar_conf_ssids *)priv;
	struct cfg80211_ssid *new_ssid = ssids->ssids;
	struct cfg80211_ssid *pos_ssid = NULL;

	Sstar_printk_debug("%s:posl(%p),posr(%p) %s\n", __func__, posl, posr,
			   posl);
	if (priv == NULL) {
		Sstar_printk_err("%s:priv NULL(%p),\n", __func__, priv);
		return false;
	}
	if (ssids->n_ssids >= SSTAR_MAX_SCAN_SSID) {
		Sstar_printk_err("%s:ssid to mach(%d)\n", __func__,
				 ssids->n_ssids);
		return false;
	}
	if ((posr - posl <= 0) || (posr - posl > IEEE80211_MAX_SSID_LEN)) {
		Sstar_printk_err("%s:channles len err\n", __func__);
		return false;
	}

	if (new_ssid && ssids->n_ssids) {
		new_ssid = Sstar_krealloc(ssids->ssids,
					  sizeof(struct cfg80211_ssid) *
						  (ssids->n_ssids + 1),
					  GFP_KERNEL);
		if (new_ssid == NULL) {
			Sstar_printk_err("%s:channles realloc err\n", __func__);
			return false;
		}
	} else {
		new_ssid =
			Sstar_kzalloc(sizeof(struct cfg80211_ssid), GFP_KERNEL);

		if (new_ssid == NULL) {
			Sstar_printk_err("%s:channles kzalloc err\n", __func__);
			return false;
		}
	}
	ssids->ssids = new_ssid;
	pos_ssid = new_ssid + ssids->n_ssids;

	memcpy(pos_ssid->ssid, posl, posr - posl);
	pos_ssid->ssid_len = posr - posl;
	ssids->n_ssids++;

	Sstar_printk_always("%s: ssid (%s) ssid_len(%d) n_ssids(%d)\n",
			    __func__, pos_ssid->ssid, pos_ssid->ssid_len,
			    ssids->n_ssids);
	return true;
}

static bool Sstar_module_config_macs(const char *posl, const char *posr,
				     void *priv)
{
	struct Sstar_conf_macs *macs = (struct Sstar_conf_macs *)priv;

	struct ieee80211_internal_mac *new_mac = macs->mac;
	struct ieee80211_internal_mac *pos_mac = NULL;
	int i;
	u8 hex;
	u8 mac_begin = 0;
	u8 mac_len = 0;

	Sstar_printk_debug("%s:posl(%p),posr(%p) %s\n", __func__, posl, posr,
			   posl);
	if (priv == NULL) {
		Sstar_printk_err("%s:priv NULL(%p),\n", __func__, priv);
		return false;
	}
	if (posr - posl != 17) {
		Sstar_printk_err("%s:mac len err(%zu),\n", __func__,
				 posr - posl);
		return false;
	}
	if (macs->n_macs >= SSTAR_MAX_SCAN_MAC_FILTER) {
		Sstar_printk_err("%s:mac to mach(%d),\n", __func__,
				 macs->n_macs);
		return false;
	}
	if (macs->n_macs && new_mac) {
		new_mac = Sstar_krealloc(macs->mac,
					 sizeof(struct ieee80211_internal_mac) *
						 (macs->n_macs + 1),
					 GFP_KERNEL);

		if (new_mac == NULL) {
			Sstar_printk_err("%s:mac realloc err\n", __func__);
			return false;
		}
	} else {
		new_mac = Sstar_kzalloc(sizeof(struct ieee80211_internal_mac),
					GFP_KERNEL);

		if (new_mac == NULL) {
			Sstar_printk_err("%s:mac kzalloc err\n", __func__);
			return false;
		}
	}

	macs->mac = new_mac;
	pos_mac = macs->mac + macs->n_macs;

	for (i = 0; i < posr - posl; i++) {
		if (posl[i] == ':') {
			mac_len++;
			mac_begin = 0;
			continue;
		}

		if (mac_len >= 6) {
			Sstar_printk_err("%s:mac len,%d\n", __func__, mac_len);
			return false;
		}

		if (mac_begin > 1) {
			Sstar_printk_err("%s:mac err,mac_begin %d\n", __func__,
					 mac_begin);
			return false;
		}
		if (Sstar_accsii_to_hex(posl[i], &hex) == false) {
			Sstar_printk_err("%s:scan_macs  %d\n", __func__,
					 posl[i]);
			return false;
		}
		pos_mac->mac[mac_len] *= 16;

		pos_mac->mac[mac_len] += hex;

		mac_begin++;
	}

	if (mac_len != 5) {
		Sstar_printk_err("%s:mac len err  %d\n", __func__, mac_len);
		return false;
	}
	macs->n_macs++;
	Sstar_printk_always("%s:mac[%pM]\n", __func__, pos_mac->mac);
	return true;
}
static bool Sstar_module_config_ies(const char *posl, const char *posr,
				    void *priv)
{
	struct Sstar_conf_ies *ies = (struct Sstar_conf_ies *)priv;
	u8 *ie_pos = NULL;
	u32 Sstar_oui = SSTAR_PRIVATE_OUI;
	Sstar_printk_debug("%s:posl(%p),posr(%p) %s\n", __func__, posl, posr,
			   posl);

	if (priv == NULL) {
		Sstar_printk_err("%s:priv NULL(%p),\n", __func__, priv);
		return false;
	}
	if (ies->ie_len) {
		Sstar_printk_err("%s:only surport one ien", __func__);
		return false;
	}
	if ((posr - posl <= 0) || (posr - posl > 255)) {
		Sstar_printk_err("%s:special len err\n", __func__);
		return false;
	}

	ies->ies = Sstar_kzalloc(posr - posl + (ies->Sstar_ie ? 6 : 0),
				 GFP_KERNEL);

	if (ies->ies == NULL) {
		Sstar_printk_err("%s:mac kzalloc err\n", __func__);
		return false;
	}
	ie_pos = ies->ies;
	if (ies->Sstar_ie) {
		*ie_pos++ = SSTAR_WLAN_EID_PRIVATE;
		*ie_pos++ = posr - posl + 4;
		memcpy(ie_pos, &Sstar_oui, 4);
		ie_pos += 4;
	}
	memcpy(ie_pos, posl, posr - posl);
	ies->ie_len = posr - posl + (ies->Sstar_ie ? 6 : 0);

	return true;
}
static bool Sstar_module_config_string(const char *posl, const char *posr,
				       void *priv)
{
	struct Sstar_config_string_hash *string_list =
		(struct Sstar_config_string_hash *)priv;
	struct Sstar_conf_string *string_node;
	int len = posr - posl;
	struct hlist_head *hhead;
	struct hlist_node *node;

	if (string_list == NULL) {
		Sstar_printk_err("%s:string_list == NULL \n", __func__);
		return false;
	}

	if (len <= 0) {
		Sstar_printk_err("%s:len <= 0\n", __func__);
		return false;
	}
	if (string_list->empty == true) {
		Sstar_printk_err("%s:string_list is empry\n", __func__);
		return false;
	}

	hhead = Sstar_hash_list(posl, len, string_list->hlist_head,
				SSTAR_COMMON_HASHBITS);

	hlist_for_each (node, hhead) {
		string_node =
			hlist_entry(node, struct Sstar_conf_string, hnode);

		if ((!strncmp(posl, string_node->strings, len)) &&
		    (strlen(string_node->strings) == len)) {
			if (string_node->set == true) {
				Sstar_printk_err("%s:[%s] has been set\n",
						 __func__,
						 string_node->strings);
				return false;
			}

			string_node->set = true;

			return true;
		}
	}
	Sstar_printk_err("%s:[%s] not found\n", __func__, posl);
	return false;
}
static void Sstar_module_config_string_hash_int(
	struct Sstar_config_string_hash *string_list)
{
	string_list->empty = true;
	Sstar_common_hash_list_init(string_list->hlist_head,
				    SSTAR_COMMON_HASHENTRIES);
}

static void Sstar_module_config_string_hash_insert(
	struct Sstar_config_string_hash *string_list,
	struct Sstar_conf_string *string_node)
{
	unsigned int hash_index = Sstar_hash_index(string_node->strings,
						   strlen(string_node->strings),
						   SSTAR_COMMON_HASHBITS);
	hlist_add_head(&string_node->hnode,
		       &string_list->hlist_head[hash_index]);
	string_list->empty = false;
}

static void Sstar_module_config_mult_string_hash(
	struct Sstar_config_string_hash *string_list, size_t n_string, ...)
{
	size_t index = 0;
	va_list args;
	struct Sstar_conf_string *string_node;

	Sstar_module_config_string_hash_int(string_list);

	va_start(args, n_string);
	for (index = 0; index < n_string; index++) {
		string_node = va_arg(args, struct Sstar_conf_string *);
		Sstar_module_config_string_hash_insert(string_list,
						       string_node);
	}
	va_end(args);
}
static bool Sstar_module_echo_enable_num_echo(const char *posl,
					      const char *posr, void *priv)
{
	struct Sstar_echo_params *echo_params =
		(struct Sstar_echo_params *)priv;
	Sstar_printk_debug("%s:posl(%p),posr(%p) %s\n", __func__, posl, posr,
			   posl);

	if (priv == NULL) {
		Sstar_printk_err("%s:priv NULL(%p),\n", __func__, priv);
		return false;
	}

	if (echo_params->num != -1) {
		Sstar_printk_err("%s:echo_params->num err(%d),\n", __func__,
				 echo_params->num);
		return false;
	}

	if ((posr - posl <= 0) || (posr - posl > 5)) {
		Sstar_printk_err("%s: params too long\n", __func__);
		return false;
	}

	if (Sstar_accsii_to_int(posl, posr - posl, &echo_params->num) ==
	    false) {
		Sstar_printk_err("%s:echo_params->num (%d) err\n", __func__,
				 echo_params->num);
		echo_params->num = -1;
		return false;
	}

	if ((echo_params->num < 0) && (echo_params->num != -1)) {
		Sstar_printk_err("%s:echo_params->num (%d) is negtive\n",
				 __func__, echo_params->num);
		echo_params->num = -1;
		return false;
	}

	return true;
}
static bool
Sstar_module_attr_echo_enable(struct Sstar_common *hw_priv,
			      struct Sstar_store_cmd_code *cmd_store,
			      const char *buf, int len)
{
	bool res = true;
	const char *info = buf;
	void *cmd_private = NULL;
	struct Sstar_echo_params *echo_params;

	enable_echo = true;
	cmd_store->echo_enable = true;

	cmd_private = Sstar_store_cmd_init_cmd_private(
		cmd_store, sizeof(struct Sstar_echo_params), GFP_KERNEL);

	if (cmd_private == NULL) {
		res = false;
		goto exit;
	}
	echo_params = (struct Sstar_echo_params *)cmd_private;
	echo_params->num = -1;
	rcu_assign_pointer(cmd_store->cmd_private, cmd_private);

	info = Sstar_skip_space(buf, len);

	/*
	*parmas start end and channels
	*/
	if (info) {
#define NUM_ECHO "num_echo="
		struct Sstar_sting_s req_string[] = {
			SSTAR_STRING_TABLE_INIT(
				NUM_ECHO, Sstar_module_echo_enable_num_echo,
				cmd_store->cmd_private),
		};
		if (Sstar_string_parase(cmd_store, req_string,
					ARRAY_SIZE(req_string), info,
					len - (info - buf)) == false) {
			Sstar_printk_always("%s:channel_select %s err\n",
					    __func__, info);
			res = false;
			goto exit;
		}
	}

	num_echo = echo_params->num;
	res = true;
	Sstar_printk_err("%s:num_echo(%d)\n", __func__, num_echo);
exit:
	if (cmd_private) {
		Sstar_kfree(cmd_private);
		rcu_assign_pointer(cmd_store->cmd_private, NULL);
	}
	return res;
}

static bool
Sstar_module_attr_echo_disable(struct Sstar_common *hw_priv,
			       struct Sstar_store_cmd_code *cmd_store,
			       const char *buf, int len)
{
	bool res = true;

	if (buf != NULL || len != 0) {
		Sstar_printk_err("%s:len(%d)(%p) err\n", __func__, len, buf);
		res = false;
		goto exit;
	}

	enable_echo = false;
	cmd_store->echo_enable = true;
exit:
	return res;
}

static bool Sstar_module_attr_scan_action_process(const char *posl,
						  const char *posr, void *priv)
{
#define POS "true"
#define PAS "false"

	struct Sstar_scan_params *scan_params =
		(struct Sstar_scan_params *)priv;
	size_t len = posr - posl;

	if (posr - posl > 5) {
		Sstar_printk_err("%s:(%zu)\n", __func__, posr - posl);
		return false;
	}

	if ((len == strlen(POS)) && (!strncmp(posl, POS, len))) {
		Sstar_printk_debug("%s:positive scan\n", __func__);
		scan_params->positive = true;
	} else if ((len == strlen(PAS)) && (!strncmp(posl, PAS, len))) {
		Sstar_printk_debug("%s:passive scan\n", __func__);
		scan_params->positive = false;
	} else {
		return false;
	}

	return true;
}
static bool
Sstar_module_attr_scan_paras_scan(struct Sstar_store_cmd_code *cmd_store,
				  const char *buf, int len,
				  struct Sstar_scan_params *scan_params)
{
#define SCAN_CHANNEL "channel="
#define SCAN_SSID "ssid="
#define SCAN_PRIVATE "private="
#define SCAN_MAC "mac="
#define SCAN_POS "positive="

	const char *scan_info = buf;

	struct Sstar_sting_s scan_string[] = {
		SSTAR_STRING_TABLE_INIT(SCAN_CHANNEL,
					Sstar_module_config_channels,
					&scan_params->channels),
		SSTAR_STRING_TABLE_INIT(SCAN_SSID, Sstar_module_config_ssids,
					&scan_params->ssids),
		SSTAR_STRING_TABLE_INIT(SCAN_PRIVATE, Sstar_module_config_ies,
					&scan_params->ies),
		SSTAR_STRING_TABLE_INIT(SCAN_MAC, Sstar_module_config_macs,
					&scan_params->macs),
		SSTAR_STRING_TABLE_INIT(SCAN_POS,
					Sstar_module_attr_scan_action_process,
					scan_params),
	};
	memset(scan_params, 0, sizeof(struct Sstar_scan_params));
	scan_info = Sstar_skip_space(buf, len);
	scan_params->ies.Sstar_ie = 1;
	if (scan_info) {
		Sstar_printk_debug("%s:scan_info %s\n", __func__, scan_info);
		if (Sstar_string_parase(cmd_store, scan_string,
					ARRAY_SIZE(scan_string), scan_info,
					len - (scan_info - buf)) == false) {
			Sstar_printk_always("%s:scan_info %s err\n", __func__,
					    scan_info);
			goto err;
		}
	}
	return true;
err:

	if (scan_params->channels.channels)
		Sstar_kfree(scan_params->channels.channels);
	if (scan_params->macs.mac)
		Sstar_kfree(scan_params->macs.mac);
	if (scan_params->ies.ies)
		Sstar_kfree(scan_params->ies.ies);
	if (scan_params->ssids.ssids)
		Sstar_kfree(scan_params->ssids.ssids);

	memset(scan_params, 0, sizeof(struct Sstar_scan_params));
	return false;
}
bool Sstar_module_attr_scan_result_handle(
	struct ieee80211_hw *hw, struct Sstar_internal_scan_results_req *req,
	struct ieee80211_internal_scan_sta *sta)
{
	struct Sstar_store_cmd_code *cmd_store = req->priv;

	Sstar_module_show_put(&cmd_store->echo, "ssid=[%s]" SSTAR_SPACE_STR,
			      (char *)sta->ssid);
	Sstar_module_show_put(&cmd_store->echo,
			      "bssid=[" SSTAR_MACSTR "]" SSTAR_SPACE_STR,
			      SSTAR_MAC2STR(sta->bssid));
	Sstar_module_show_put(&cmd_store->echo, "signal=[%d]" SSTAR_SPACE_STR,
			      sta->signal);
	Sstar_module_show_put(&cmd_store->echo,
			      "channel=[%d]" SSTAR_SPACE_STR SSTAR_LINEF_STR,
			      sta->channel);
	req->n_stas++;

	return true;
}
static bool Sstar_module_attr_scan(struct Sstar_common *hw_priv,
				   struct Sstar_store_cmd_code *cmd_store,
				   const char *buf, int len)
{
	struct Sstar_scan_params scan_params;
	struct ieee80211_sub_if_data *sdata = NULL;
	struct ieee80211_sub_if_data *sdata_tmp = NULL;
	struct ieee80211_local *local = hw_to_local(hw_priv->hw);
	struct ieee80211_internal_scan_request internal_scan;
	struct Sstar_internal_scan_results_req results_req;
	int i = 0;
	bool res = true;

	Sstar_printk_debug("%s:%d:%s\n", __func__, len, buf);

	memset(&scan_params, 0, sizeof(struct Sstar_scan_params));
	scan_params.ies.Sstar_ie = 1;

	rcu_assign_pointer(cmd_store->cmd_private, NULL);
	if (Sstar_module_attr_scan_paras_scan(cmd_store, buf, len,
					      &scan_params) == false) {
		Sstar_printk_always("%s scan params err\n", __func__);
		res = false;
		goto err_params;
	}

	rtnl_lock();

	list_for_each_entry (sdata_tmp, &local->interfaces, list) {
		if (sdata_tmp->vif.type != NL80211_IFTYPE_STATION) {
			continue;
		}

		if (!ieee80211_sdata_running(sdata_tmp)) {
			continue;
		}

		sdata = sdata_tmp;
		break;
	}
	if (sdata == NULL) {
		Sstar_printk_err("%s:no staion running\n", __func__);
		res = false;
		goto err;
	}
	Sstar_printk_always("triger [%s] to scan\n", sdata->name);
	memset(&internal_scan, 0,
	       sizeof(struct ieee80211_internal_scan_request));

	if (scan_params.ssids.n_ssids) {
		for (i = 0; i < scan_params.ssids.n_ssids; i++) {
			Sstar_printk_debug("%s: ssid[%s][%d]\n", __func__,
					   scan_params.ssids.ssids[i].ssid,
					   scan_params.ssids.ssids[i].ssid_len);
		}
	}
	if (scan_params.macs.n_macs) {
		for (i = 0; i < scan_params.macs.n_macs; i++) {
			Sstar_printk_debug("%s: mac[%pM]\n", __func__,
					   scan_params.macs.mac[i].mac);
		}
	}
	if (scan_params.channels.n_channels) {
		for (i = 0; i < scan_params.channels.n_channels; i++) {
			Sstar_printk_debug("%s: channel[%d]\n", __func__,
					   scan_params.channels.channels[i]);
		}
	}
	internal_scan.macs = scan_params.macs.mac;
	internal_scan.n_macs = scan_params.macs.n_macs;
	internal_scan.channels = scan_params.channels.channels;
	internal_scan.n_channels = scan_params.channels.n_channels;
	internal_scan.ssids = scan_params.ssids.ssids;
	internal_scan.n_ssids = scan_params.ssids.n_ssids;
	internal_scan.ies = scan_params.ies.ies;
	internal_scan.ie_len = scan_params.ies.ie_len;
	internal_scan.no_cck = true;

	rcu_assign_pointer(internal_scan.result_handle, NULL);
	rcu_assign_pointer(internal_scan.priv, NULL);
	if (Sstar_internal_cmd_scan_triger(sdata, &internal_scan) == false) {
		res = false;
		goto err;
	}
	Sstar_printk_always("%s\n", __func__);
	Sstar_printk_always("%s:channel(%p)\n", __func__,
			    scan_params.channels.channels);
	memset(&results_req, 0, sizeof(struct Sstar_internal_scan_results_req));

	results_req.flush = true;
	results_req.priv = cmd_store;
	results_req.result_handle = Sstar_module_attr_scan_result_handle;

	if (ieee80211_scan_internal_req_results(sdata->local, &results_req) ==
	    false) {
		Sstar_printk_err("%s:scan results err\n", __func__);
		res = false;
		goto err;
	}
	res = true;
err:
	rtnl_unlock();
err_params:
	if (scan_params.channels.channels)
		Sstar_kfree(scan_params.channels.channels);
	if (scan_params.ies.ies)
		Sstar_kfree(scan_params.ies.ies);
	if (scan_params.macs.mac)
		Sstar_kfree(scan_params.macs.mac);
	if (scan_params.ssids.ssids)
		Sstar_kfree(scan_params.ssids.ssids);
	return res;
}

static bool
Sstar_module_attr_start_monitor(struct Sstar_common *hw_priv,
				struct Sstar_store_cmd_code *cmd_store,
				const char *buf, int len)
{
#define MONITOR_CHANNEL "channel="
#define MONITOR_CHTYPE "channeltype="
	struct ieee80211_sub_if_data *sdata = NULL;
	struct ieee80211_sub_if_data *sdata_tmp = NULL;
	struct ieee80211_local *local = hw_to_local(hw_priv->hw);
	struct Sstar_start_monitor_params monitor_params;
	const char *monitor_info = buf;
	struct ieee80211_internal_monitor_req req;
	bool res = true;

	memset(&monitor_params, 0, sizeof(struct Sstar_start_monitor_params));
	memset(&req, 0, sizeof(struct ieee80211_internal_monitor_req));

	monitor_info = Sstar_skip_space(buf, len);

	if (monitor_info) {
		struct Sstar_conf_string ht20 = { .strings = "HT20",
						  .set = false };
		struct Sstar_conf_string ht40 = { .strings = "HT40",
						  .set = false };
		struct Sstar_config_string_hash string_hash;
		struct Sstar_sting_s monitor_string[] = {
			SSTAR_STRING_TABLE_INIT(MONITOR_CHANNEL,
						Sstar_module_config_channels,
						&monitor_params.channel),
			SSTAR_STRING_TABLE_INIT(MONITOR_CHTYPE,
						Sstar_module_config_string,
						&string_hash),
		};

		Sstar_module_config_string_hash_int(&string_hash);
		Sstar_module_config_string_hash_insert(&string_hash, &ht20);
		Sstar_module_config_string_hash_insert(&string_hash, &ht40);

		Sstar_printk_debug("%s:scan_info %s\n", __func__, monitor_info);
		if (Sstar_string_parase(cmd_store, monitor_string,
					ARRAY_SIZE(monitor_string),
					monitor_info,
					len - (monitor_info - buf)) == false) {
			Sstar_printk_err("%s:monitor %s err\n", __func__,
					 monitor_info);
			res = false;
			goto exit_nolock;
		}

		if (monitor_params.channel.n_channels != 1) {
			Sstar_printk_err("%s:channel %d ++++err\n", __func__,
					 monitor_params.channel.n_channels);
			res = false;
			goto exit_nolock;
		}

		req.ch = monitor_params.channel.channels[0];
		req.chtype = 0;
	} else {
		Sstar_printk_err("%s:monitor need channel\n", __func__);
		goto exit_nolock;
	}

	if (req.ch == 0) {
		goto exit_nolock;
	}
	rtnl_lock();

	list_for_each_entry (sdata_tmp, &local->interfaces, list) {
		if (sdata_tmp->vif.type == NL80211_IFTYPE_MONITOR) {
			Sstar_printk_err("%s:monitor already exit\n", __func__);
			res = false;
			goto exit;
		}

		if (!ieee80211_sdata_running(sdata_tmp)) {
			continue;
		}

		sdata = sdata_tmp;
		break;
	}

	if (sdata == NULL) {
		Sstar_printk_err("%s:no staion running\n", __func__);
		res = false;
		goto exit;
	}

	res = Sstar_internal_cmd_monitor_req(sdata, &req);

	Sstar_printk_debug("triger [%s] to monitor (%d)\n", sdata->name, res);
exit:
	rtnl_unlock();
exit_nolock:
	if (monitor_params.channel.channels)
		Sstar_kfree(monitor_params.channel.channels);

	return res;
}
static bool
Sstar_module_attr_stop_monitor(struct Sstar_common *hw_priv,
			       struct Sstar_store_cmd_code *cmd_store,
			       const char *buf, int len)
{
	struct ieee80211_sub_if_data *sdata = NULL;
	struct ieee80211_sub_if_data *sdata_tmp = NULL;
	struct ieee80211_local *local = hw_to_local(hw_priv->hw);
	bool res = true;
	const char *monitor_info = buf;

	monitor_info = Sstar_skip_space(buf, len);

	if (monitor_info) {
		Sstar_printk_debug("%s:scan_info %s\n", __func__, monitor_info);
		if (Sstar_string_parase(cmd_store, NULL, 0, monitor_info,
					len - (monitor_info - buf)) == false) {
			Sstar_printk_always("%s:stop monitor %s err\n",
					    __func__, monitor_info);
			goto exit;
		}
	}

	rtnl_lock();

	list_for_each_entry (sdata_tmp, &local->interfaces, list) {
		if (sdata_tmp->vif.type != NL80211_IFTYPE_MONITOR) {
			Sstar_printk_err("%s:monitor already exit\n", __func__);
			continue;
		}

		if (!ieee80211_sdata_running(sdata_tmp)) {
			continue;
		}

		sdata = sdata_tmp;
		break;
	}

	if (sdata == NULL) {
		rtnl_unlock();
		Sstar_printk_err("%s:no staion running\n", __func__);
		res = false;
		goto exit;
	}

	res = Sstar_internal_cmd_stop_monitor(sdata);

	Sstar_printk_debug("triger [%s] to monitor (%d)\n", sdata->name, res);

	rtnl_unlock();
exit:
	return res;
}
static ssize_t
Sstar_module_attr_stop_monitor_show(struct Sstar_common *hw_priv,
				    struct Sstar_store_cmd_code *cmd_store,
				    char *buf, bool show)
{
	return Sstar_module_attr_default_show(hw_priv, cmd_store, buf, show);
}

static ssize_t
Sstar_module_attr_start_monitor_show(struct Sstar_common *hw_priv,
				     struct Sstar_store_cmd_code *cmd_store,
				     char *buf, bool show)
{
	return Sstar_module_attr_default_show(hw_priv, cmd_store, buf, show);
}

static bool
Sstar_module_sta_info_insert(struct Sstar_store_cmd_code *cmd_store,
			     struct ieee80211_internal_sta_info *stainfo)
{
	Sstar_module_show_put(&cmd_store->echo, "ifname[%s]" SSTAR_SPACE_STR,
			      stainfo->sdata->name);
	Sstar_module_show_put(&cmd_store->echo, "mac[%pM]" SSTAR_SPACE_STR,
			      stainfo->mac);
	if (IEEE80211_INTERNAL_STA_FLAGS_SSID & stainfo->filled) {
		Sstar_module_show_put(&cmd_store->echo,
				      "ssid[%s]" SSTAR_SPACE_STR,
				      stainfo->ssid);
		Sstar_module_show_put(&cmd_store->echo,
				      "ssid_len[%d]" SSTAR_SPACE_STR,
				      stainfo->ssid_len);
	}
	if (IEEE80211_INTERNAL_STA_FLAGS_CHANNEL & stainfo->filled) {
		Sstar_module_show_put(&cmd_store->echo,
				      "channel[%d]" SSTAR_SPACE_STR,
				      stainfo->channel);
		Sstar_module_show_put(&cmd_store->echo,
				      "channel_type[%d]" SSTAR_SPACE_STR,
				      stainfo->channel_type);
	}
	if (IEEE80211_INTERNAL_STA_FLAGS_SIGNAL & stainfo->filled) {
		Sstar_module_show_put(&cmd_store->echo,
				      "signal[%d]" SSTAR_SPACE_STR,
				      stainfo->signal);
		Sstar_module_show_put(&cmd_store->echo,
				      "avg_signal[%d]" SSTAR_SPACE_STR,
				      stainfo->avg_signal);
	}
	if (IEEE80211_INTERNAL_STA_FLAGS_TXRXBYTE & stainfo->filled) {
		Sstar_module_show_put(&cmd_store->echo,
				      "rxbytes[%ld]" SSTAR_SPACE_STR,
				      stainfo->rx_bytes);
		Sstar_module_show_put(&cmd_store->echo,
				      "txbytes[%ld]" SSTAR_SPACE_STR,
				      stainfo->tx_bytes);
	}
	if (IEEE80211_INTERNAL_STA_FLAGS_TOPRATE & stainfo->filled) {
		Sstar_module_show_put(&cmd_store->echo,
				      "toprate[%d]" SSTAR_SPACE_STR,
				      stainfo->top_rate);
	}

	Sstar_module_show_put(&cmd_store->echo, SSTAR_LINEF_STR);
	return true;
}
static bool
Sstar_module_ap_info_handle(struct ieee80211_internal_sta_info *stainfo,
			    void *priv)
{
	struct Sstar_store_cmd_code *cmd_store =
		(struct Sstar_store_cmd_code *)priv;

	if (cmd_store == NULL) {
		return false;
	}

	if (stainfo->sdata->vif.type != NL80211_IFTYPE_STATION) {
		return false;
	}
	Sstar_module_sta_info_insert(cmd_store, stainfo);
	return true;
}
static bool
Sstar_module_sta_info_req_spilt(struct Sstar_store_cmd_code *cmd_store,
				const char *buf, int len)
{
#define REQ_MAC "mac="
#define REQ_INFO "req="

	struct Sstar_stainfo_req_params *req = cmd_store->cmd_private;
	struct Sstar_conf_string rssi = { .strings = "rssi", .set = false };
	struct Sstar_conf_string toprate = { .strings = "toprate",
					     .set = false };
	struct Sstar_conf_string channel = { .strings = "channel",
					     .set = false };
	struct Sstar_conf_string txrxbytes = { .strings = "txrxbytes",
					       .set = false };
	struct Sstar_conf_string ssid = { .strings = "ssid", .set = false };
	struct Sstar_config_string_hash string_hash;

	struct Sstar_sting_s req_string[] = {
		SSTAR_STRING_TABLE_INIT(REQ_MAC, Sstar_module_config_macs,
					&req->req_mac),
		SSTAR_STRING_TABLE_INIT(REQ_INFO, Sstar_module_config_string,
					&string_hash),
	};

	Sstar_module_config_string_hash_int(&string_hash);
	Sstar_module_config_string_hash_insert(&string_hash, &rssi);
	Sstar_module_config_string_hash_insert(&string_hash, &toprate);
	Sstar_module_config_string_hash_insert(&string_hash, &channel);
	Sstar_module_config_string_hash_insert(&string_hash, &txrxbytes);
	Sstar_module_config_string_hash_insert(&string_hash, &ssid);

	if (Sstar_string_parase(cmd_store, req_string, ARRAY_SIZE(req_string),
				buf, len) == false) {
		goto err;
	}

	if (rssi.set)
		req->req_rssi = 1;
	if (toprate.set)
		req->req_rate = 1;
	if (channel.set)
		req->req_channel = 1;
	if (txrxbytes.set)
		req->req_txrxbytes = 1;
	if (ssid.set)
		req->req_ssid = 1;

	return true;
err:
	if (req) {
		if (req->req_mac.mac) {
			Sstar_kfree(req->req_mac.mac);
		}
		req->req_mac.mac = NULL;
		req->req_mac.n_macs = 0;
	}
	return false;
}
static bool Sstar_module_attr_ap_info(struct Sstar_common *hw_priv,
				      struct Sstar_store_cmd_code *cmd_store,
				      const char *buf, int len)
{
	struct ieee80211_internal_sta_req req;
	bool res = true;
	const char *ap_info = buf;
	void *cmd_private = NULL;
	struct Sstar_stainfo_req_params *req_params = NULL;

	cmd_private = Sstar_store_cmd_init_cmd_private(
		cmd_store, sizeof(struct Sstar_stainfo_req_params), GFP_KERNEL);

	if (cmd_private == NULL) {
		goto err;
	}

	rcu_assign_pointer(cmd_store->cmd_private, cmd_private);
	ap_info = Sstar_skip_space(buf, len);

	req_params = cmd_private;
	req_params->req_channel = false;
	req_params->req_rate = false;
	req_params->req_rssi = false;
	req_params->req_txrxbytes = false;
	req_params->type = NL80211_IFTYPE_STATION;

	if (ap_info) {
		Sstar_printk_debug("%s:ap_info %s\n", __func__, ap_info);
		if (Sstar_module_sta_info_req_spilt(cmd_store, ap_info,
						    len - (ap_info - buf)) ==
		    false) {
			Sstar_printk_always("%s:ap_info %s err\n", __func__,
					    ap_info);
			res = false;
			goto err;
		}
	}
	memset(&req, 0, sizeof(struct ieee80211_internal_sta_req));

	req.sta_handle = Sstar_module_ap_info_handle;
	req.priv = cmd_store;

	if (req_params->req_channel)
		req.req_flag |= IEEE80211_INTERNAL_STA_FLAGS_CHANNEL;
	if (req_params->req_rssi)
		req.req_flag |= IEEE80211_INTERNAL_STA_FLAGS_SIGNAL;
	if (req_params->req_txrxbytes)
		req.req_flag |= IEEE80211_INTERNAL_STA_FLAGS_TXRXBYTE;
	if (req_params->req_rate)
		req.req_flag |= IEEE80211_INTERNAL_STA_FLAGS_TOPRATE;
	if (req_params->req_ssid)
		req.req_flag |= IEEE80211_INTERNAL_STA_FLAGS_SSID;
	if (req.req_flag == 0)
		req.req_flag = (u32)-1;

	req.n_macs = req_params->req_mac.n_macs;
	req.macs = req_params->req_mac.mac;
	req.type = NL80211_IFTYPE_STATION;
	res = Sstar_internal_cmd_stainfo(hw_to_local(hw_priv->hw), &req);
err:
	if (req_params) {
		if (req_params->req_mac.mac)
			Sstar_kfree(req_params->req_mac.mac);
		Sstar_kfree(req_params);
		rcu_assign_pointer(cmd_store->cmd_private, NULL);
	}
	return res;
}
static bool
Sstar_module_sta_info_handle(struct ieee80211_internal_sta_info *stainfo,
			     void *priv)
{
	struct Sstar_store_cmd_code *cmd_store =
		(struct Sstar_store_cmd_code *)priv;

	if (cmd_store == NULL) {
		return false;
	}

	if (stainfo->sdata->vif.type != NL80211_IFTYPE_AP) {
		return false;
	}

	Sstar_module_sta_info_insert(cmd_store, stainfo);
	return false;
}

static bool Sstar_module_attr_sta_info(struct Sstar_common *hw_priv,
				       struct Sstar_store_cmd_code *cmd_store,
				       const char *buf, int len)
{
	struct ieee80211_internal_sta_req req;
	bool res = true;
	const char *sta_info = buf;
	void *cmd_private = NULL;
	struct Sstar_stainfo_req_params *req_params = NULL;

	cmd_private = Sstar_store_cmd_init_cmd_private(
		cmd_store, sizeof(struct Sstar_stainfo_req_params), GFP_KERNEL);

	if (cmd_private == NULL) {
		goto err;
	}

	rcu_assign_pointer(cmd_store->cmd_private, cmd_private);
	req_params = cmd_private;
	req_params->type = NL80211_IFTYPE_AP;
	req_params->req_channel = false;
	req_params->req_rate = false;
	req_params->req_rssi = false;
	req_params->req_txrxbytes = false;
	sta_info = Sstar_skip_space(buf, len);

	if (sta_info) {
		Sstar_printk_debug("%s:scan_info %s\n", __func__, sta_info);
		if (Sstar_module_sta_info_req_spilt(cmd_store, sta_info,
						    len - (sta_info - buf)) ==
		    false) {
			Sstar_printk_always("%s:scan_info %s err\n", __func__,
					    sta_info);
			res = false;
			goto err;
		}
	}
	memset(&req, 0, sizeof(struct ieee80211_internal_sta_req));

	req.sta_handle = Sstar_module_sta_info_handle;
	req.priv = cmd_store;

	if (req_params->req_channel)
		req.req_flag |= IEEE80211_INTERNAL_STA_FLAGS_CHANNEL;
	if (req_params->req_rssi)
		req.req_flag |= IEEE80211_INTERNAL_STA_FLAGS_SIGNAL;
	if (req_params->req_txrxbytes)
		req.req_flag |= IEEE80211_INTERNAL_STA_FLAGS_TXRXBYTE;
	if (req_params->req_rate)
		req.req_flag |= IEEE80211_INTERNAL_STA_FLAGS_TOPRATE;
	if (req_params->req_ssid)
		req.req_flag |= IEEE80211_INTERNAL_STA_FLAGS_SSID;
	if (req.req_flag == 0)
		req.req_flag = (u32)-1;

	req.type = NL80211_IFTYPE_AP;
	req.n_macs = req_params->req_mac.n_macs;
	req.macs = req_params->req_mac.mac;
	res = Sstar_internal_cmd_stainfo(hw_to_local(hw_priv->hw), &req);
err:
	if (req_params) {
		if (req_params->req_mac.mac)
			Sstar_kfree(req_params->req_mac.mac);
		Sstar_kfree(req_params);
		rcu_assign_pointer(cmd_store->cmd_private, NULL);
	}
	return res;
}
static bool Sstar_module_wsm_adaptive_process(const char *posl,
					      const char *posr, void *priv)
{
#define ADAPTIVE_ON "on"
#define ADAPTIVE_OFF "off"
	struct Sstar_wsm_req_parmas *wsm_req =
		(struct Sstar_wsm_req_parmas *)priv;
	u8 len = posr - posl;

	Sstar_printk_debug("%s:posl(%p),posr(%p) %s\n", __func__, posl, posr,
			   posl);

	if (posl >= posr) {
		Sstar_printk_err("%s:posr(%p),posl(%p)\n", __func__, posr,
				 posl);
		return false;
	}

	if (wsm_req->req_adaptive != 0) {
		Sstar_printk_err("%s:double req return err\n", __func__);
		return false;
	}

	wsm_req->req_adaptive |= PARAMAS_SET;

	if ((strlen(ADAPTIVE_ON) == len) &&
	    (memcmp(ADAPTIVE_ON, posl, len) == 0)) {
		wsm_req->req_adaptive |= PARAMAS_ON;
		Sstar_printk_debug("adaptive on\n");
	} else if ((strlen(ADAPTIVE_OFF) == len) &&
		   (memcmp(ADAPTIVE_OFF, posl, len) == 0)) {
		wsm_req->req_adaptive &= ~PARAMAS_ON;
		Sstar_printk_debug("adaptive off\n");
	} else {
		Sstar_printk_err("%s:err %s\n", __func__, posl);
		return false;
	}

	return true;
}

static bool Sstar_module_wsm_txpwr_dcxo_process(const char *posl,
						const char *posr, void *priv)
{
	struct Sstar_wsm_req_parmas *wsm_req =
		(struct Sstar_wsm_req_parmas *)priv;
	u8 len = posr - posl;

	Sstar_printk_debug("%s:posl(%p),posr(%p) %s\n", __func__, posl, posr,
			   posl);

	if (posl >= posr) {
		Sstar_printk_err("%s:posr(%p),posl(%p)\n", __func__, posr,
				 posl);
		return false;
	}

	if (wsm_req->dcxo_index >= 4) {
		Sstar_printk_err("%s:wsm_req->dcxo_index err (%d)\n", __func__,
				 wsm_req->dcxo_index);
		return false;
	}

	wsm_req->req_pwrdcxo |= PARAMAS_SET;

	if (Sstar_accsii_to_int(posl, len,
				&wsm_req->txpwr_dcxo[wsm_req->dcxo_index]) ==
	    false) {
		Sstar_printk_err("%s:dcxo vall err %s\n", __func__, posl);
		return false;
	}

	wsm_req->dcxo_index++;
	return true;
}

static bool Sstar_module_wsm_txpwr_process(const char *posl, const char *posr,
					   void *priv)
{
	struct Sstar_wsm_req_parmas *wsm_req =
		(struct Sstar_wsm_req_parmas *)priv;
	u8 len = posr - posl;

	Sstar_printk_debug("%s:posl(%p),posr(%p) %s\n", __func__, posl, posr,
			   posl);

	if (posl >= posr) {
		Sstar_printk_err("%s:posr(%p),posl(%p)\n", __func__, posr,
				 posl);
		return false;
	}

	if (wsm_req->req_pwr != 0) {
		Sstar_printk_err("%s:double req return err\n", __func__);
		return false;
	}

	wsm_req->req_pwr |= PARAMAS_SET;

	if (Sstar_accsii_to_int(posl, len, &wsm_req->txpwr) == false) {
		Sstar_printk_err("%s:txpwr vall err %s\n", __func__, posl);
		return false;
	}
	Sstar_printk_debug("%s:txpwr(%d)\n", __func__, wsm_req->txpwr);
	return true;
}

/*
*wsm cmd:wsm_cmd adaptive=[on or off] txpwr_dcxo=[M][H][L][dcxo] txpwr=[power]
*/
static bool
Sstar_module_wsm_cmd_info_spilt(struct Sstar_store_cmd_code *cmd_store,
				const char *buf, int len)
{
#define REQ_ADAPTIVE "adaptive="
#define REQ_TXPWR_DCXO "txpwr_dcxo="
#define REQ_TXPWR "txpwr="
	bool res = true;

	struct Sstar_sting_s req_string[] = {
		SSTAR_STRING_TABLE_INIT(REQ_ADAPTIVE,
					Sstar_module_wsm_adaptive_process,
					cmd_store->cmd_private),
		SSTAR_STRING_TABLE_INIT(REQ_TXPWR_DCXO,
					Sstar_module_wsm_txpwr_dcxo_process,
					cmd_store->cmd_private),
		SSTAR_STRING_TABLE_INIT(REQ_TXPWR,
					Sstar_module_wsm_txpwr_process,
					cmd_store->cmd_private),
	};

	if (Sstar_string_parase(cmd_store, req_string, ARRAY_SIZE(req_string),
				buf, len) == false) {
		res = false;
	}

	return res;
}
static bool Sstar_module_attr_wsm_cmd(struct Sstar_common *hw_priv,
				      struct Sstar_store_cmd_code *cmd_store,
				      const char *buf, int len)
{
	const char *wsm_info = NULL;
	struct Sstar_wsm_req_parmas *wsm_req;
	void *cmd_private = NULL;
	bool res = true;

	cmd_private = Sstar_store_cmd_init_cmd_private(
		cmd_store, sizeof(struct Sstar_wsm_req_parmas), GFP_KERNEL);

	if (cmd_private == NULL) {
		res = false;
		goto err;
	}

	wsm_req = (struct Sstar_wsm_req_parmas *)cmd_private;
	rcu_assign_pointer(cmd_store->cmd_private, cmd_private);
	wsm_req->req_adaptive = 0;
	wsm_req->req_pwr = 0;
	wsm_req->req_pwrdcxo = 0;

	wsm_info = Sstar_skip_space(buf, len);

	if (wsm_info == NULL) {
		res = false;
		goto err;
	}

	res = Sstar_module_wsm_cmd_info_spilt(cmd_store, buf, len);

	if (res == false) {
		goto err;
	}

	if (wsm_req->req_adaptive) {
		struct ieee80211_internal_wsm_adaptive adaptive;

		adaptive.enable = !!(wsm_req->req_adaptive & PARAMAS_ON);

		res = Sstar_internal_wsm_adaptive(hw_priv, &adaptive);

		if (res == false) {
			Sstar_printk_err("%s:set adaptive err\n", __func__);
			goto err;
		}
	}

	if (wsm_req->req_pwrdcxo) {
		struct ieee80211_internal_wsm_txpwr_dcxo txpwr_dcxo;

		if (wsm_req->dcxo_index != 4) {
			Sstar_printk_err("%s:set req_pwrdcxo too short\n",
					 __func__);
			goto err;
		}
		txpwr_dcxo.txpwr_L = wsm_req->txpwr_dcxo[0];
		txpwr_dcxo.txpwr_M = wsm_req->txpwr_dcxo[1];
		txpwr_dcxo.txpwr_H = wsm_req->txpwr_dcxo[2];
		txpwr_dcxo.dcxo = wsm_req->txpwr_dcxo[3];

		res = Sstar_internal_wsm_txpwr_dcxo(hw_priv, &txpwr_dcxo);

		if (res == false) {
			Sstar_printk_err("%s:set txpwr_dcxo err\n", __func__);
			goto err;
		}
	}
	if (wsm_req->req_pwr) {
		struct ieee80211_internal_wsm_txpwr txpwr;

		txpwr.txpwr_indx = wsm_req->txpwr;

		res = Sstar_internal_wsm_txpwr(hw_priv, &txpwr);

		if (res == false) {
			Sstar_printk_err("%s:set txpwr err\n", __func__);
			goto err;
		}
	}
err:
	if (cmd_private) {
		Sstar_kfree(cmd_private);
		rcu_assign_pointer(cmd_store->cmd_private, NULL);
	}

	return res;
}
static bool
Sstar_module_send_probe_request_spilt(struct Sstar_store_cmd_code *cmd_store,
				      const char *buf, int len)
{
#define REQ_NUM "num="
#define REQ_IE "private="
	bool res = true;
	struct Sstar_probe_request *request =
		(struct Sstar_probe_request *)cmd_store->cmd_private;

	struct Sstar_sting_s req_string[] = {
		SSTAR_STRING_TABLE_INIT(REQ_NUM, Sstar_module_config_integers,
					&request->n_request),
		SSTAR_STRING_TABLE_INIT(REQ_IE, Sstar_module_config_ies,
					&request->ies),
	};

	if (Sstar_string_parase(cmd_store, req_string, ARRAY_SIZE(req_string),
				buf, len) == false) {
		res = false;
	}

	if (request->n_request.n_integers == 0) {
		request->n_request.integers =
			Sstar_kzalloc(sizeof(int), GFP_KERNEL);
		if (request->n_request.integers != NULL) {
			request->n_request.integers[0] = 1;
			request->n_request.n_integers = 1;
		} else {
			res = false;
		}
	} else if (request->n_request.n_integers > 1) {
		res = false;
	}
	return res;
}

static bool
Sstar_module_attr_send_probe_request(struct Sstar_common *hw_priv,
				     struct Sstar_store_cmd_code *cmd_store,
				     const char *buf, int len)
{
	void *cmd_private = NULL;
	struct Sstar_probe_request *request = NULL;
	bool res = true;
	const char *request_info = NULL;
	struct ieee80211_sub_if_data *sdata = NULL;
	struct ieee80211_sub_if_data *sdata_tmp = NULL;
	struct ieee80211_local *local = hw_to_local(hw_priv->hw);
	int i = 0;

	cmd_private = Sstar_store_cmd_init_cmd_private(
		cmd_store, sizeof(struct Sstar_probe_request), GFP_KERNEL);

	if (cmd_private == NULL) {
		res = false;
		goto err;
	}

	request = (struct Sstar_probe_request *)cmd_private;
	rcu_assign_pointer(cmd_store->cmd_private, cmd_private);

	request_info = Sstar_skip_space(buf, len);

	if (request_info == NULL) {
		res = false;
		goto err;
	}

	res = Sstar_module_send_probe_request_spilt(cmd_store, buf, len);

	if (res == false) {
		goto err;
	}

	rtnl_lock();

	list_for_each_entry (sdata_tmp, &local->interfaces, list) {
		if ((sdata_tmp->vif.type != NL80211_IFTYPE_MONITOR) &&
		    (sdata_tmp->vif.type != NL80211_IFTYPE_STATION)) {
			Sstar_printk_err("%s:monitor already exit\n", __func__);
			continue;
		}

		if (!ieee80211_sdata_running(sdata_tmp)) {
			continue;
		}

		if (sdata_tmp->vif.type == NL80211_IFTYPE_STATION) {
			struct Sstar_vif *priv =
				(struct Sstar_vif *)sdata_tmp->vif.drv_priv;

			if (atomic_read(&priv->enabled) == 0)
				continue;
			if (priv->join_status !=
			    SSTAR_APOLLO_JOIN_STATUS_STA_LISTEN)
				continue;
		}
		sdata = sdata_tmp;
		break;
	}

	if (sdata == NULL) {
		rtnl_unlock();
		Sstar_printk_err("%s:no staion running\n", __func__);
		res = false;
		goto err;
	}

	Sstar_printk_err(
		"triger [%s] to monitor/sta to send probe(%d),ie_len(%d)\n",
		sdata->name, request->n_request.integers[0],
		request->ies.ie_len);
	for (i = 0; i < request->n_request.integers[0]; i++) {
		ieee80211_send_special_probe_req(sdata, NULL, NULL, 0,
						 request->ies.ies,
						 request->ies.ie_len);
	}

	rtnl_unlock();
err:
	if (cmd_private) {
		request = cmd_private;
		if (request->ies.ies)
			Sstar_kfree(request->ies.ies);
		if (request->n_request.integers)
			Sstar_kfree(request->n_request.integers);
		Sstar_kfree(cmd_private);

		rcu_assign_pointer(cmd_store->cmd_private, NULL);
	}
	return res;
}
#ifdef CONFIG_SSTAR_STA_LISTEN
static bool
Sstar_module_sta_listen_spilt(struct Sstar_store_cmd_code *cmd_store,
			      const char *buf, int len)
{
#define LISTEN_IFNAME "ifname="
#define LISTEN_CHANNEL "channel="
	bool res = true;
	struct Sstar_sta_listen *sta_listen =
		(struct Sstar_sta_listen *)cmd_store->cmd_private;
	struct Sstar_sting_s req_string[] = {
		SSTAR_STRING_TABLE_INIT(LISTEN_IFNAME,
					Sstar_module_config_ifname,
					&sta_listen->ifname),
		SSTAR_STRING_TABLE_INIT(LISTEN_CHANNEL,
					Sstar_module_config_channels,
					&sta_listen->channel),
	};

	if (Sstar_string_parase(cmd_store, req_string, ARRAY_SIZE(req_string),
				buf, len) == false) {
		res = false;
	}

	if (sta_listen->ifname.size == 0)
		res = false;
	if (sta_listen->channel.n_channels != 1)
		res = false;
	return res;
}

static bool Sstar_module_attr_sta_listen(struct Sstar_common *hw_priv,
					 struct Sstar_store_cmd_code *cmd_store,
					 const char *buf, int len)
{
	struct Sstar_sta_listen *sta_listen;
	void *cmd_private = NULL;
	bool res = true;
	const char *request_info = NULL;

	struct ieee80211_sub_if_data *sdata = NULL;
	struct ieee80211_sub_if_data *sdata_tmp = NULL;
	struct ieee80211_local *local = hw_to_local(hw_priv->hw);

	cmd_private = Sstar_store_cmd_init_cmd_private(
		cmd_store, sizeof(struct Sstar_sta_listen), GFP_KERNEL);

	if (cmd_private == NULL) {
		goto err;
	}

	sta_listen = (struct Sstar_sta_listen *)cmd_private;

	rcu_assign_pointer(cmd_store->cmd_private, cmd_private);

	request_info = Sstar_skip_space(buf, len);
	if (request_info == NULL) {
		res = false;
		goto err;
	}

	res = Sstar_module_sta_listen_spilt(cmd_store, buf, len);

	if (res == false) {
		goto err;
	}
	rtnl_lock();

	list_for_each_entry (sdata_tmp, &local->interfaces, list) {
		if (sdata_tmp->vif.type != NL80211_IFTYPE_STATION) {
			Sstar_printk_err("%s:not sta\n", __func__);
			continue;
		}

		if (!ieee80211_sdata_running(sdata_tmp)) {
			continue;
		}

		if (memcmp(sdata_tmp->name, sta_listen->ifname.if_name,
			   IFNAMSIZ)) {
			continue;
		}
		sdata = sdata_tmp;
		break;
	}

	if (sdata == NULL) {
		rtnl_unlock();
		Sstar_printk_err("%s:no staion running\n", __func__);
		res = false;
		goto err;
	}

	Sstar_printk_err("%s:[%s] enter sta listen mode\n", __func__,
			 sdata->name);
	if (ieee80211_set_sta_channel(sdata, sta_listen->channel.channels[0]) !=
	    0)
		res = false;
	rtnl_unlock();
err:
	if (cmd_private) {
		if (sta_listen->channel.channels)
			Sstar_kfree(sta_listen->channel.channels);
		Sstar_kfree(cmd_private);
		rcu_assign_pointer(cmd_store->cmd_private, NULL);
	}

	return res;
}
#endif

static bool
Sstar_module_ap_set_beacon_spilt(struct Sstar_store_cmd_code *cmd_store,
				 const char *buf, int len)
{
#define AP_IFNAME "ifname="
#define AP_SPECIAL "special="
	bool res = true;
	struct Sstar_ap_set_beacon *ap_beacon =
		(struct Sstar_ap_set_beacon *)cmd_store->cmd_private;
	struct Sstar_sting_s req_string[] = {
		SSTAR_STRING_TABLE_INIT(AP_IFNAME, Sstar_module_config_ifname,
					&ap_beacon->ifname),
		SSTAR_STRING_TABLE_INIT(AP_SPECIAL, Sstar_module_config_ies,
					&ap_beacon->ies),
	};

	if (Sstar_string_parase(cmd_store, req_string, ARRAY_SIZE(req_string),
				buf, len) == false) {
		res = false;
	}

	return res;
}

static bool
Sstar_module_attr_ap_set_beacon(struct Sstar_common *hw_priv,
				struct Sstar_store_cmd_code *cmd_store,
				const char *buf, int len)
{
	struct Sstar_ap_set_beacon *ap_beacon = NULL;
	void *cmd_private = NULL;
	bool res = true;
	const char *request_info = NULL;

	struct ieee80211_sub_if_data *sdata = NULL;
	struct ieee80211_sub_if_data *sdata_tmp = NULL;
	struct ieee80211_local *local = hw_to_local(hw_priv->hw);

	cmd_private = Sstar_store_cmd_init_cmd_private(
		cmd_store, sizeof(struct Sstar_ap_set_beacon), GFP_KERNEL);

	if (cmd_private == NULL) {
		goto err;
	}

	ap_beacon = (struct Sstar_ap_set_beacon *)cmd_private;
	rcu_assign_pointer(cmd_store->cmd_private, cmd_private);

	request_info = Sstar_skip_space(buf, len);
	if (request_info == NULL) {
		res = false;
		goto err;
	}

	res = Sstar_module_ap_set_beacon_spilt(cmd_store, buf, len);

	if (res == false) {
		goto err;
	}

	if (ap_beacon->ifname.size == 0) {
		Sstar_printk_err("%s: ifname not set\n", __func__);
		res = false;
		goto err;
	}

	if (ap_beacon->ies.ie_len == 0) {
		Sstar_printk_err("%s: special ie not set\n", __func__);
		res = false;
		goto err;
	}
	rtnl_lock();
	list_for_each_entry (sdata_tmp, &local->interfaces, list) {
		if (sdata_tmp->vif.type != NL80211_IFTYPE_AP) {
			continue;
		}

		if (!ieee80211_sdata_running(sdata_tmp)) {
			continue;
		}

		if (memcmp(sdata_tmp->name, ap_beacon->ifname.if_name,
			   IFNAMSIZ)) {
			continue;
		}
		sdata = sdata_tmp;
		break;
	}

	if (sdata == NULL) {
		rtnl_unlock();
		Sstar_printk_err("%s:[%s] is not running\n", __func__,
				 ap_beacon->ifname.if_name);
		res = false;
		goto err;
	}
	rtnl_unlock();
	Sstar_printk_debug("%s:[%s],len[%zu] to beacon\n", __func__,
			   ap_beacon->ifname.if_name,
			   (size_t)ap_beacon->ies.ie_len);
	res = ieee80211_ap_update_special_beacon(sdata, ap_beacon->ies.ies,
						 ap_beacon->ies.ie_len);
err:
	if (ap_beacon) {
		if (ap_beacon->ies.ies)
			Sstar_kfree(ap_beacon->ies.ies);
		ap_beacon->ies.ies = NULL;
		Sstar_kfree(ap_beacon);
		rcu_assign_pointer(cmd_store->cmd_private, NULL);
	}

	return res;
}
static bool
Sstar_module_attr_sta_special_scan(struct Sstar_common *hw_priv,
				   struct Sstar_store_cmd_code *cmd_store,
				   const char *buf, int len)
{
	bool res = true;
	struct Sstar_scan_params scan_params;
	struct ieee80211_sub_if_data *sdata = NULL;
	struct ieee80211_sub_if_data *sdata_tmp = NULL;
	struct ieee80211_local *local = hw_to_local(hw_priv->hw);
	int i = 0;

	Sstar_printk_debug("%s:%d:%s\n", __func__, len, buf);
	rtnl_lock();

	memset(&scan_params, 0, sizeof(struct Sstar_scan_params));

	scan_params.ies.Sstar_ie = 1;
	if (Sstar_module_attr_scan_paras_scan(cmd_store, buf, len,
					      &scan_params) == false) {
		Sstar_printk_always("%s scan params err\n", __func__);
		res = false;
		goto err;
	}

	list_for_each_entry (sdata_tmp, &local->interfaces, list) {
		if (sdata_tmp->vif.type != NL80211_IFTYPE_STATION) {
			continue;
		}

		if (!ieee80211_sdata_running(sdata_tmp)) {
			continue;
		}

		sdata = sdata_tmp;
		break;
	}
	if (sdata == NULL) {
		res = false;
		Sstar_printk_err("%s:no staion running\n", __func__);
		goto err;
	}
	Sstar_printk_always("triger [%s] to scan\n", sdata->name);

	if (scan_params.ssids.n_ssids) {
		for (i = 0; i < scan_params.ssids.n_ssids; i++) {
			Sstar_printk_debug("%s: ssid[%s][%d]\n", __func__,
					   scan_params.ssids.ssids[i].ssid,
					   scan_params.ssids.ssids[i].ssid_len);
		}
	}
	if (scan_params.macs.n_macs) {
		for (i = 0; i < scan_params.macs.n_macs; i++) {
			Sstar_printk_debug("%s: mac[%pM]\n", __func__,
					   scan_params.macs.mac[i].mac);
		}
	}
	if (scan_params.channels.n_channels) {
		for (i = 0; i < scan_params.channels.n_channels; i++) {
			Sstar_printk_debug("%s: channel[%d]\n", __func__,
					   scan_params.channels.channels[i]);
		}
	}

	Sstar_printk_always("%s\n", __func__);
	Sstar_printk_always("%s:channel(%p)\n", __func__,
			    scan_params.channels.channels);

	if (scan_params.positive == true) {
		u8 *scan_ssid = NULL;
		size_t scan_ssid_len = 0;

		if (scan_params.ssids.n_ssids) {
			scan_ssid = scan_params.ssids.ssids[0].ssid;
			scan_ssid_len = scan_params.ssids.ssids[0].ssid_len;
		}
		res = ieee80211_sta_triger_positive_scan(
			sdata, scan_params.channels.channels,
			scan_params.channels.n_channels, scan_ssid,
			scan_ssid_len, scan_params.ies.ies,
			scan_params.ies.ie_len, NULL);
	} else {
		res = ieee80211_sta_triger_passive_scan(
			sdata, scan_params.channels.channels,
			scan_params.channels.n_channels);
	}
err:
	rtnl_unlock();
	if (scan_params.channels.channels)
		Sstar_kfree(scan_params.channels.channels);
	if (scan_params.ies.ies)
		Sstar_kfree(scan_params.ies.ies);
	if (scan_params.macs.mac)
		Sstar_kfree(scan_params.macs.mac);
	if (scan_params.ssids.ssids)
		Sstar_kfree(scan_params.ssids.ssids);
	return res;
}
static bool Sstar_module_attr_set_freq(struct Sstar_common *hw_priv,
				       struct Sstar_store_cmd_code *cmd_store,
				       const char *buf, int len)
{
#define FREQ_CHANNEL "channel="
#define FREQ_FREQ "freq="
	struct Sstar_special_freq *set_freq;
	void *cmd_private = NULL;
	bool res = true;
	const char *request_info = NULL;
	struct ieee80211_internal_set_freq_req req;

	cmd_private = Sstar_store_cmd_init_cmd_private(
		cmd_store, sizeof(struct Sstar_special_freq), GFP_KERNEL);

	if (cmd_private == NULL) {
		res = false;
		goto err;
	}

	set_freq = (struct Sstar_special_freq *)cmd_private;

	rcu_assign_pointer(cmd_store->cmd_private, cmd_private);

	request_info = Sstar_skip_space(buf, len);
	if (request_info == NULL) {
		res = false;
		goto err;
	}
	/*
	*parase channel and freq
	*/
	if (1) {
		struct Sstar_sting_s req_string[] = {
			SSTAR_STRING_TABLE_INIT(FREQ_CHANNEL,
						Sstar_module_config_integers,
						&set_freq->channel),
			SSTAR_STRING_TABLE_INIT(FREQ_FREQ,
						Sstar_module_config_integers,
						&set_freq->freq),
		};
		res = Sstar_string_parase(cmd_store, req_string,
					  ARRAY_SIZE(req_string), buf, len);
		if (res == false) {
			Sstar_printk_err("%s:Sstar_string_parase err\n",
					 __func__);
			goto err;
		}
	}

	if (set_freq->channel.n_integers != 1) {
		Sstar_printk_err("%s: channel not set\n", __func__);
		res = false;
		goto err;
	}

	if (set_freq->freq.n_integers != 1) {
		Sstar_printk_err("%s: freq not set\n", __func__);
		res = false;
		goto err;
	}

	req.channel_num = set_freq->channel.integers[0];
	req.freq = set_freq->freq.integers[0];
	req.set = true;
	rtnl_lock();
	res = Sstar_internal_freq_set(hw_priv->hw, &req);
	rtnl_unlock();
err:
	if (cmd_private) {
		if (set_freq->channel.integers)
			Sstar_kfree(set_freq->channel.integers);
		if (set_freq->freq.integers)
			Sstar_kfree(set_freq->freq.integers);
		Sstar_kfree(cmd_private);
		rcu_assign_pointer(cmd_store->cmd_private, NULL);
	}
#undef FREQ_CHANNEL
#undef FREQ_FREQ
	return res;
}
static bool Sstar_module_attr_clear_freq(struct Sstar_common *hw_priv,
					 struct Sstar_store_cmd_code *cmd_store,
					 const char *buf, int len)
{
#define FREQ_CHANNEL "channel="
	struct Sstar_special_freq *set_freq;
	void *cmd_private = NULL;
	bool res = true;
	const char *request_info = NULL;
	struct ieee80211_internal_set_freq_req req;

	cmd_private = Sstar_store_cmd_init_cmd_private(
		cmd_store, sizeof(struct Sstar_special_freq), GFP_KERNEL);

	if (cmd_private == NULL) {
		res = false;
		goto err;
	}

	set_freq = (struct Sstar_special_freq *)cmd_private;

	rcu_assign_pointer(cmd_store->cmd_private, cmd_private);

	request_info = Sstar_skip_space(buf, len);
	if (request_info == NULL) {
		res = false;
		goto err;
	}
	/*
	*parase channel
	*/
	if (1) {
		struct Sstar_sting_s req_string[] = {
			SSTAR_STRING_TABLE_INIT(FREQ_CHANNEL,
						Sstar_module_config_integers,
						&set_freq->channel),
		};
		res = Sstar_string_parase(cmd_store, req_string,
					  ARRAY_SIZE(req_string), buf, len);
		if (res == false) {
			Sstar_printk_err("%s:Sstar_string_parase err\n",
					 __func__);
			goto err;
		}
	}

	if (set_freq->channel.n_integers != 1) {
		Sstar_printk_err("%s: channel not set\n", __func__);
		res = false;
		goto err;
	}

	req.channel_num = set_freq->channel.integers[0];
	req.set = false;

	rtnl_lock();
	res = Sstar_internal_freq_set(hw_priv->hw, &req);
	rtnl_unlock();
err:
	if (cmd_private) {
		if (set_freq->channel.integers)
			Sstar_kfree(set_freq->channel.integers);
		Sstar_kfree(cmd_private);
		rcu_assign_pointer(cmd_store->cmd_private, NULL);
	}
#undef FREQ_CHANNEL
#undef FREQ_FREQ
	return res;
}
static ssize_t
Sstar_module_attr_channel_select_show(struct Sstar_common *hw_priv,
				      struct Sstar_store_cmd_code *cmd_store,
				      char *buf, bool show)
{
	return Sstar_module_attr_default_show(hw_priv, cmd_store, buf, show);
}
static bool
Sstar_module_attr_channel_select(struct Sstar_common *hw_priv,
				 struct Sstar_store_cmd_code *cmd_store,
				 const char *buf, int len)
{
#define CH_START "start="
#define CH_END "end="
#define CH_LIST "channels="
#define CH_VER "version="
	struct Sstar_channel_auto_select *select = NULL;
	struct ieee80211_sub_if_data *sdata = NULL;
	struct ieee80211_sub_if_data *sdata_tmp = NULL;
	struct ieee80211_local *local = hw_to_local(hw_priv->hw);
	struct ieee80211_internal_channel_auto_select_req req;
	struct ieee80211_internal_channel_auto_select_results results;
	//	u8 ignor_channels[2]; // = {13,14};
	bool res = true;
	const char *select_info = buf;
	void *cmd_private = NULL;
	int i;

	rtnl_lock();
	cmd_private = Sstar_store_cmd_init_cmd_private(
		cmd_store, sizeof(struct Sstar_channel_auto_select),
		GFP_KERNEL);

	if (cmd_private == NULL) {
		res = false;
		goto exit;
	}
	select = (struct Sstar_channel_auto_select *)cmd_private;

	rcu_assign_pointer(cmd_store->cmd_private, cmd_private);

	select_info = Sstar_skip_space(buf, len);

	/*
	*parmas start end and channels
	*/
	if (select_info) {
		struct Sstar_sting_s req_string[] = {
			SSTAR_STRING_TABLE_INIT(CH_START,
						Sstar_module_config_integers,
						&select->start_channel),
			SSTAR_STRING_TABLE_INIT(CH_END,
						Sstar_module_config_integers,
						&select->end_channel),
			SSTAR_STRING_TABLE_INIT(CH_LIST,
						Sstar_module_config_channels,
						&select->channels),
			SSTAR_STRING_TABLE_INIT(CH_VER,
						Sstar_module_config_integers,
						&select->version),
		};
		if (Sstar_string_parase(cmd_store, req_string,
					ARRAY_SIZE(req_string), select_info,
					len - (select_info - buf)) == false) {
			Sstar_printk_always("%s:channel_select %s err\n",
					    __func__, select_info);
			res = false;
			goto exit;
		}
	}
	/*
	*get channel list
	*if set channel list , using the channel setting
	*if set channel start and end , using the channel between start and end
	*if not set channel ,using all surported channel
	*/
	if (select->end_channel.n_integers ^ select->start_channel.n_integers) {
		Sstar_printk_err("%s: start or end err\n", __func__);
		res = false;
		goto exit;
	}

	if (select->channels.n_channels) {
		if (select->end_channel.n_integers) {
			Sstar_printk_err(
				"%s: channel list and start_end both set\n",
				__func__);
			res = false;
			goto exit;
		}
	}
	if (select->channels.n_channels) {
		for (i = 0; i < select->channels.n_channels; i++) {
			if (ieee8011_channel_valid(
				    hw_priv->hw,
				    select->channels.channels[i]) == false) {
				res = false;
				goto exit;
			}
		}
	} else if (select->end_channel.n_integers) {
		if (select->end_channel.n_integers != 1) {
			Sstar_printk_err("%s:end err\n", __func__);
			res = false;
			goto exit;
		}

		if (select->start_channel.n_integers != 1) {
			Sstar_printk_err("%s:end err\n", __func__);
			res = false;
			goto exit;
		}

		if (select->end_channel.integers[0] <=
		    select->start_channel.integers[0]) {
			res = false;
			goto exit;
		}

		if (select->end_channel.integers[0] -
			    select->start_channel.integers[0] >=
		    IEEE80211_SSTAR_MAX_SCAN_CHANNEL_INDEX) {
			res = false;
			goto exit;
		}
		select->channels.channels = Sstar_kmalloc(
			select->end_channel.integers[0] -
				select->start_channel.integers[0] + 1,
			GFP_KERNEL);

		if (select->channels.channels == NULL) {
			res = false;
			goto exit;
		}
		select->channels.n_channels =
			select->end_channel.integers[0] -
			select->start_channel.integers[0] + 1;
		for (i = 0; i < select->channels.n_channels; i++) {
			if (ieee8011_channel_valid(
				    hw_priv->hw,
				    select->start_channel.integers[0] + i) ==
			    false) {
				Sstar_printk_err(
					"%s:select->start_channel errL\n",
					__func__);
				goto exit;
			}
			select->channels.channels[i] =
				select->start_channel.integers[0] + i;
		}
	} else {
		select->channels.channels = ieee8011_get_channel_list(
			hw_priv->hw, (size_t *)&select->channels.n_channels);

		if (select->channels.channels == NULL) {
			Sstar_printk_err("%s: select->channels == NULL\n",
					 __func__);
			res = false;
			goto exit;
		}
	}

	list_for_each_entry (sdata_tmp, &local->interfaces, list) {
		if (sdata_tmp->vif.type != NL80211_IFTYPE_STATION) {
			continue;
		}

		if (!ieee80211_sdata_running(sdata_tmp)) {
			continue;
		}

		sdata = sdata_tmp;
		break;
	}

	if (sdata == NULL) {
		Sstar_printk_err("%s:no staion running\n", __func__);
		res = false;
		goto exit;
	}

	memset(&req, 0,
	       sizeof(struct ieee80211_internal_channel_auto_select_req));
	memset(&results, 0,
	       sizeof(struct ieee80211_internal_channel_auto_select_results));

	results.ignore_channels = NULL; //ignor_channels;
	results.ignore_n_channels = 0; //ARRAY_SIZE(ignor_channels);
	results.channels = select->channels.channels;
	results.n_channels = select->channels.n_channels;
	if (select->version.n_integers == 0)
		results.version = 1;
	else if (select->version.n_integers == 1)
		results.version = select->version.integers[0];
	else {
		Sstar_printk_err("%s:version err\n", __func__);
		res = false;
		goto exit;
	}
	/*
	*triger channel scan
	*/
	if (Sstar_internal_channel_auto_select(sdata, &req) == false) {
		Sstar_printk_err("%s:channel  auto select err\n", __func__);
		res = false;
		goto exit;
	}
	/*
	*get results
	*/
	if (Sstar_internal_channel_auto_select_results(sdata, &results) ==
	    false) {
		Sstar_printk_err("%s:channel results err\n", __func__);
		res = false;
		goto exit;
	}

	Sstar_module_show_put(&cmd_store->echo,
			      "suggest_channel=[%d]" SSTAR_LINEF_STR,
			      results.susgest_channel);
	for (i = 0; i < select->channels.n_channels; i++) {
		struct ieee80211_channel *ch = ieee8011_chnum_to_channel(
			hw_priv->hw, select->channels.channels[i]);
		Sstar_module_show_put(
			&cmd_store->echo,
			"channel[%d] ap_count[%d] ratio[%d] weight[%d] special_freq[%s]" SSTAR_LINEF_STR,
			select->channels.channels[i],
			results.n_aps[select->channels.channels[i] - 1],
			results.busy_ratio[select->channels.channels[i] - 1],
			results.weight[select->channels.channels[i] - 1],
			channel_in_special(ch) == true ? "true" : "false");
	}
	Sstar_printk_debug("triger [%s] auto channel select (%d)\n",
			   sdata->name, res);
exit:
#undef CH_START
#undef CH_END
#undef CH_LIST
#undef CH_VER
	rtnl_unlock();
	if (cmd_private) {
		if (select->channels.channels)
			Sstar_kfree(select->channels.channels);
		if (select->start_channel.integers)
			Sstar_kfree(select->start_channel.integers);
		if (select->end_channel.integers)
			Sstar_kfree(select->end_channel.integers);
		if (select->version.integers)
			Sstar_kfree(select->version.integers);
		Sstar_kfree(cmd_private);
		rcu_assign_pointer(cmd_store->cmd_private, NULL);
	}
	return res;
}
static bool
Sstar_module_attr_chip_version(struct Sstar_common *hw_priv,
			       struct Sstar_store_cmd_code *cmd_store,
			       const char *buf, int len)
{
	const char *select_info = buf;
	bool res = true;
	struct ieee80211_internal_req_chip chip_info;
	select_info = Sstar_skip_space(buf, len);

	memset(&chip_info, 0, sizeof(struct ieee80211_internal_req_chip));

	if (select_info) {
		if (Sstar_string_parase(cmd_store, NULL, 0, select_info,
					len - (select_info - buf)) == false) {
			Sstar_printk_always("%s:chip_version %s err\n",
					    __func__, select_info);
			res = false;
			goto exit;
		}
	}

	chip_info.flags |= IEEE80211_INTERNAL_REQ_CHIP_FLAGS__CHIP_VER;

	if (Sstar_internal_request_chip_cap(hw_priv->hw, &chip_info) == false) {
		Sstar_printk_always("%s: get chip_version\n", __func__);
		res = false;
		goto exit;
	}

	if (chip_info.chip_version == NULL)
		chip_info.chip_version = "unkown";

	Sstar_module_show_put(&cmd_store->echo, "chip_version[%s]\n",
			      chip_info.chip_version);
exit:

	return res;
}
#ifdef CONFIG_SSTAR_SUPPORT_AP_CONFIG
/*
*set ap configs:ap_config ifname=[xxx] channel=[x] ssid=[ap] bssid=[xx:xx:xx:xx:xx:xx]
*/
static bool
Sstar_module_attr_set_ap_config(struct Sstar_common *hw_priv,
				struct Sstar_store_cmd_code *cmd_store,
				const char *buf, int len)
{
	bool res = true;
	struct Sstar_module_ap_conf ap_config;
	const char *select_info = buf;
	struct ieee80211_sub_if_data *sdata = NULL;
	struct ieee80211_internal_ap_conf conf_req;

	rtnl_lock();

	select_info = Sstar_skip_space(buf, len);
	memset(&ap_config, 0, sizeof(struct Sstar_module_ap_conf));
	memset(&conf_req, 0, sizeof(struct ieee80211_internal_ap_conf));

	if (select_info) {
#define CONF_IFNAME "ifname="
#define CONF_CHANNEL "channel="
#define CONF_SSID "ssid="
#define CONF_BSSID "bssid="
#define CONF_IES "ies="

		struct Sstar_sting_s req_string[] = {
			SSTAR_STRING_TABLE_INIT(CONF_IFNAME,
						Sstar_module_config_ifname,
						&ap_config.ifname),
			SSTAR_STRING_TABLE_INIT(CONF_CHANNEL,
						Sstar_module_config_channels,
						&ap_config.channel),
			SSTAR_STRING_TABLE_INIT(CONF_SSID,
						Sstar_module_config_ssids,
						&ap_config.ssid),
			SSTAR_STRING_TABLE_INIT(CONF_BSSID,
						Sstar_module_config_macs,
						&ap_config.bssid),
			SSTAR_STRING_TABLE_INIT(CONF_IES,
						Sstar_module_config_ies, NULL),
		};

		if (Sstar_string_parase(cmd_store, req_string,
					ARRAY_SIZE(req_string), select_info,
					len - (select_info - buf)) == false) {
			Sstar_printk_always("%s:ap config %s err\n", __func__,
					    select_info);
			res = false;
			goto exit;
		}
	} else {
		res = false;
		goto exit;
	}

	if (ap_config.ifname.size == 0) {
		Sstar_printk_err("%s:ifname is null\n", __func__);
		res = false;
		goto exit;
	}

	sdata = Sstar_module_get_sdata_by_name(ap_config.ifname.if_name);

	if (sdata == NULL) {
		Sstar_printk_err("%s:[%s] not found\n", __func__,
				 ap_config.ifname.if_name);
		res = false;
		goto exit;
	}

	if ((ap_config.channel.n_channels > 1) ||
	    (ap_config.ssid.n_ssids > 1) || (ap_config.bssid.n_macs > 1)) {
		Sstar_printk_err("%s:config err\n", __func__);
		res = false;
		goto exit;
	}

	if ((ap_config.channel.n_channels == 0) &&
	    (ap_config.ssid.n_ssids == 0) && (ap_config.bssid.n_macs == 0)) {
		Sstar_printk_err("%s:config clear\n", __func__);
		res = Sstar_internal_update_ap_conf(sdata, NULL, true);
		goto exit;
	}

	Sstar_printk_debug("n_channels(%d),n_macs(%d),n_ssids(%d)\n",
			   ap_config.channel.n_channels, ap_config.bssid.n_macs,
			   ap_config.ssid.n_ssids);
	if (ap_config.channel.n_channels)
		conf_req.channel = ap_config.channel.channels[0];
	if (ap_config.bssid.n_macs)
		memcpy(conf_req.bssid, ap_config.bssid.mac[0].mac, 6);
	if (ap_config.ssid.n_ssids) {
		memcpy(conf_req.ssid, ap_config.ssid.ssids[0].ssid,
		       ap_config.ssid.ssids[0].ssid_len);
		conf_req.ssid_len = ap_config.ssid.ssids[0].ssid_len;
	}
	res = Sstar_internal_update_ap_conf(sdata, &conf_req, false);
exit:
	rtnl_unlock();
#undef CONF_IFNAME
#undef CONF_CHANNEL
#undef CONF_SSID
#undef CONF_BSSID
	if (ap_config.bssid.mac)
		Sstar_kfree(ap_config.bssid.mac);
	if (ap_config.channel.channels)
		Sstar_kfree(ap_config.channel.channels);
	if (ap_config.ssid.ssids)
		Sstar_kfree(ap_config.ssid.ssids);
	rcu_assign_pointer(cmd_store->cmd_private, NULL);
	return res;
}
#endif
/*
*set_mode:ap_config ifname=[xxx] mode=[managed/monior] channel=[x]
*/

static bool Sstar_module_attr_set_mode(struct Sstar_common *hw_priv,
				       struct Sstar_store_cmd_code *cmd_store,
				       const char *buf, int len)
{
#define MODE_IFNAME "ifname="
#define MODE_CHANNEL "channel="
#define MODE_TYPE "mode="
	struct Sstar_module_set_mode_conf conf_mode;
	const char *select_info = buf;
	struct ieee80211_sub_if_data *sdata = NULL;
	struct ieee80211_internal_iftype_req req;
	struct Sstar_config_string_hash string_hash;
	struct Sstar_conf_string managed = { .strings = "managed",
					     .set = false };
	struct Sstar_conf_string monitor = { .strings = "monitor",
					     .set = false };
	struct Sstar_sting_s monitor_string[] = {
		SSTAR_STRING_TABLE_INIT(MODE_IFNAME, Sstar_module_config_ifname,
					&conf_mode.ifname),
		SSTAR_STRING_TABLE_INIT(MODE_CHANNEL,
					Sstar_module_config_channels,
					&conf_mode.channel),
		SSTAR_STRING_TABLE_INIT(MODE_TYPE, Sstar_module_config_string,
					&string_hash),
	};
	bool res = true;

	rtnl_lock();

	select_info = Sstar_skip_space(buf, len);

	if (select_info == NULL) {
		Sstar_printk_err("%s:need params\n", __func__);
		res = false;

		goto exit;
	}
	memset(&req, 0, sizeof(struct ieee80211_internal_iftype_req));
	memset(&conf_mode, 0, sizeof(struct Sstar_module_set_mode_conf));
	Sstar_module_config_mult_string_hash(&string_hash, 2, &managed,
					     &monitor);

	if (Sstar_string_parase(cmd_store, monitor_string,
				ARRAY_SIZE(monitor_string), select_info,
				len - (select_info - buf)) == false) {
		Sstar_printk_always("%s:ap config %s err\n", __func__,
				    select_info);
		res = false;
		goto exit;
	}

	if (conf_mode.ifname.size == 0) {
		Sstar_printk_err("%s:ifname is null\n", __func__);
		res = false;
		goto exit;
	}

	if (((managed.set == true) ^ (monitor.set == true)) == 0) {
		Sstar_printk_err("%s:mode is null\n", __func__);
		res = false;
		goto exit;
	}

	if (conf_mode.channel.n_channels > 1) {
		Sstar_printk_always("%s:n_channels %d err\n", __func__,
				    conf_mode.channel.n_channels);
		res = false;
		goto exit;
	}

	sdata = Sstar_module_get_sdata_by_name(conf_mode.ifname.if_name);

	if (sdata == NULL) {
		Sstar_printk_err("%s:[%s] not found\n", __func__,
				 conf_mode.ifname.if_name);
		res = false;
		goto exit;
	}
	if (conf_mode.channel.n_channels == 1)
		req.channel = conf_mode.channel.channels[0];
	if (managed.set == true)
		req.if_type = IEEE80211_INTERNAL_IFTYPE_REQ__MANAGED;
	else if (monitor.set == true)
		req.if_type = IEEE80211_INTERNAL_IFTYPE_REQ__MONITOR;

	res = Sstar_internal_cmd_req_iftype(sdata, &req);
exit:
	if (conf_mode.channel.channels)
		Sstar_kfree(conf_mode.channel.channels);
	rtnl_unlock();
#undef MODE_IFNAME
#undef MODE_CHANNEL
#undef MODE_TYPE
	return res;
}
static bool Sstar_module_attr_halt(struct Sstar_common *hw_priv,
				   struct Sstar_store_cmd_code *cmd_store,
				   const char *buf, int len)
{
#define HALT_IFNAME "ifname="
	bool res = true;
	struct Sstar_module_halt_conf halt_conf;
	const char *select_info = buf;
	struct Sstar_sting_s halt_string[] = {
		SSTAR_STRING_TABLE_INIT(HALT_IFNAME, Sstar_module_config_ifname,
					&halt_conf.ifname),
	};
	struct ieee80211_sub_if_data *sdata = NULL;

	rtnl_lock();
	memset(&halt_conf, 0, sizeof(struct Sstar_module_halt_conf));
	select_info = Sstar_skip_space(buf, len);
	if (select_info == NULL) {
		Sstar_printk_err("%s:need params\n", __func__);
		res = false;

		goto exit;
	}

	if (Sstar_string_parase(cmd_store, halt_string, ARRAY_SIZE(halt_string),
				select_info,
				len - (select_info - buf)) == false) {
		Sstar_printk_always("%s:ap config %s err\n", __func__,
				    select_info);
		res = false;
		goto exit;
	}

	if (halt_conf.ifname.size == 0) {
		Sstar_printk_err("%s:ifname is null\n", __func__);
		res = false;
		goto exit;
	}

	sdata = Sstar_module_get_sdata_by_name(halt_conf.ifname.if_name);

	if (sdata == NULL) {
		Sstar_printk_err("%s:[%s] not found\n", __func__,
				 halt_conf.ifname.if_name);
		res = false;
		goto exit;
	}

	Sstar_bh_halt(sdata->local->hw.priv);
exit:
	rtnl_unlock();
	return res;
#undef HALT_IFNAME
}
static bool Sstar_store_cmd_is_ready(struct Sstar_store_cmd_code *cmd_store)
{
	struct Sstar_store_cmd_code *ready_cmd;

	list_for_each_entry (ready_cmd, &Sstar_cmd_echo_ready_list, lhead) {
		if ((ready_cmd->label == cmd_store->label) &&
		    (ready_cmd->code_cmd == cmd_store->code_cmd))
			return true;
	}

	return false;
}
static void Sstar_store_cmd_echo_done(struct Sstar_store_cmd_code *cmd_store)
{
	char *cmd_private;
	if (cmd_store->echo_ready == false) {
		Sstar_printk_always("%s not in ready list\n", cmd_store->label);
		WARN_ON(Sstar_store_cmd_is_ready(cmd_store) == true);
	} else {
		list_del(&cmd_store->lhead);
		ready_list_num--;
		WARN_ON(ready_list_num < 0);
	}
	cmd_private = rcu_dereference(cmd_store->cmd_private);
	rcu_assign_pointer(cmd_store->cmd_private, NULL);
	if (cmd_private) {
		synchronize_rcu();
		Sstar_kfree(cmd_private);
		Sstar_printk_always("%s:free cmd_private(%p)\n", __func__,
				    cmd_private);
	}
	cmd_store->cmd_private = NULL;
	cmd_store->echo_ready = false;
}
static void Sstar_store_cmd_echo_ready(struct Sstar_store_cmd_code *cmd_store)
{
	if (cmd_store->echo_ready == true) {
		Sstar_printk_always("%s already in ready list\n",
				    cmd_store->label);
		WARN_ON(Sstar_store_cmd_is_ready(cmd_store) == false);
		return;
	}

	cmd_store->echo_ready = true;

	if ((num_echo == -1) || (num_echo > ready_list_num)) {
	} else if ((!list_empty(&Sstar_cmd_echo_ready_list))) {
		struct Sstar_store_cmd_code *cmd_code =
			list_first_entry(&Sstar_cmd_echo_ready_list,
					 struct Sstar_store_cmd_code, lhead);
		if (cmd_code->show_cmd)
			cmd_code->show_cmd(Sstar_hw_priv_dereference(),
					   cmd_code, NULL, false);
		Sstar_store_cmd_echo_done(cmd_code);
		Sstar_store_cmd_deinit_show(&cmd_code->echo);
	} else if (num_echo != 0) {
		WARN_ON(1);
	}
	ready_list_num++;
	list_add_tail(&cmd_store->lhead, &Sstar_cmd_echo_ready_list);
}

static ssize_t Sstar_module_decode_common_store(const char *buf, size_t n)
{
	const char *p;
	const char *code_end = NULL;
	int code_len = 0;
	int left_len = 0;
	size_t len = n;
	struct Sstar_store_cmd_code *store_code;

	p = Sstar_skip_space(buf, n);

	if (p == NULL) {
		Sstar_printk_always("%s:all space %s\n", __func__, buf);
		return -EINVAL;
	}
	len -= p - buf;
	/*
	*find the postion of cmd end,use space between tow string
	*some cmd not need space ,so find enter or end of line
	*/
	code_end = memchr(buf, SSTAR_SPACE, len);
	if (code_end == NULL) {
		code_end = memchr(buf, SSTAR_ENTER, len);
		if (code_end == NULL)
			code_end = memchr(buf, SSTAR_LINEF, len);
	}
	if (code_end == NULL) {
		Sstar_printk_always("%s:can not fine aline\n", __func__);
	}
	code_len = code_end ? code_end - p : len;
	if (code_len <= 0) {
		Sstar_printk_always("%s code_len err\n", __func__);
		return -EINVAL;
	}
	left_len = len - code_len;

	if (left_len < 0) {
		Sstar_printk_always("%s left_len err\n", __func__);
		return -EINVAL;
	}
	store_code = Sstar_store_cmd_find_cmd(p, code_len);

	if (store_code == NULL) {
		Sstar_printk_always("%s:can not find cmd\n", __func__);
		return -EINVAL;
	}
	BUG_ON(store_code->code_cmd == NULL);

	p = Sstar_skip_space(code_end, left_len);
	if (p == NULL) {
		left_len = 0;
	} else {
		left_len -= p - code_end;
	}
	if (left_len < 0) {
		Sstar_printk_always("%s left_len err,params err\n", __func__);
		return -EINVAL;
	}
	if (Sstar_store_cmd_init_show(&store_code->echo) == false) {
		Sstar_printk_always("%s show buff init err\n", __func__);
		return -EINVAL;
	}
	/*
	*set default enable_echo
	*/
	store_code->echo_enable = enable_echo;

	if (store_code->code_cmd(Sstar_hw_priv_dereference(), store_code, p,
				 left_len) == true) {
		if (store_code->echo_enable == true) {
			Sstar_module_show_put(&store_code->echo, "%s[%s]\n",
					      store_code->label, "OK");
			Sstar_store_cmd_echo_ready(store_code);
		} else {
			if (store_code->show_cmd)
				store_code->show_cmd(
					Sstar_hw_priv_dereference(), store_code,
					NULL, false);
			Sstar_store_cmd_echo_done(store_code);
			Sstar_store_cmd_deinit_show(&store_code->echo);
		}
	} else {
		if (store_code->echo_enable == true) {
			Sstar_module_show_put(&store_code->echo, "%s[%s]\n",
					      store_code->label, "ERR");
		}
		Sstar_store_cmd_echo_done(store_code);
		Sstar_store_cmd_deinit_show(&store_code->echo);

		return -EINVAL;
	}
	return n;
}
static ssize_t Sstar_module_cmd_store(struct kobject *kobj,
				      struct kobj_attribute *attr,
				      const char *buf, size_t n)
{
	ssize_t mg_size = 0;
	Sstar_modulefs_lock();
	Sstar_module_muxlock();
	if (Sstar_hw_priv_dereference() == NULL) {
		mg_size = -EINVAL;
		goto exit;
	}
	mg_size = Sstar_module_decode_common_store(buf, n);
exit:
	Sstar_module_muxunlock();
	Sstar_modulefs_unlock();
	return mg_size;
}

static u8 Sstar_write_backup_commond(char *show_buffer)
{
	u8 i = 0;
	char *ptr = NULL;
	char back_command[][10] = { { "ap_info" }, { "NULL" } };
	for (i = 0; strncmp(back_command[i], "NULL", 4) != 0; i++) {
		ptr = strstr(show_buffer, back_command[i]);
		if (ptr)
			return 1;
	}
	return 0;
}
static ssize_t Sstar_module_cmd_show(struct kobject *kobj,
				     struct kobj_attribute *attr, char *buf)
{
	ssize_t len = 0;
	u8 *show_buff = NULL;
	ssize_t show_len = 0;
	Sstar_modulefs_lock();
	Sstar_module_muxlock();
	show_buff = Sstar_kzalloc(SSTAR_SHOW_MSG_MAX_SIZE, GFP_KERNEL);
	while (!list_empty(&Sstar_cmd_echo_ready_list)) {
		struct Sstar_store_cmd_code *cmd_code =
			list_first_entry(&Sstar_cmd_echo_ready_list,
					 struct Sstar_store_cmd_code, lhead);

		if (cmd_code->show_cmd)
			show_len =
				cmd_code->show_cmd(Sstar_hw_priv_dereference(),
						   cmd_code, show_buff,
						   show_buff ? true : false);
		if (show_buff) {
			if ((show_len) &&
			    (len + show_len < SSTAR_SHOW_MSG_MAX_SIZE)) {
				memcpy(buf + len, show_buff, show_len);
				len += show_len;
			}
		}
		show_len = 0;
		Sstar_store_cmd_echo_done(cmd_code);
		Sstar_store_cmd_deinit_show(&cmd_code->echo);
	}
	if (len && show_buff && Sstar_write_backup_commond(show_buff)) {
		if (WARN_ON(len > SSTAR_SHOW_MSG_MAX_SIZE))
			len = SSTAR_SHOW_MSG_MAX_SIZE;
		memset(backup_show, 0, SSTAR_SHOW_MSG_MAX_SIZE);
		memcpy(backup_show, show_buff, len);
		backup_show_size = len;
	}
	Sstar_module_muxunlock();
	Sstar_modulefs_unlock();
	if (show_buff)
		Sstar_kfree(show_buff);
	return len;
}

static void Sstar_module_cmd_init(void)
{
	unsigned int index = 0;

	INIT_LIST_HEAD(&Sstar_cmd_echo_ready_list);
	Sstar_common_hash_list_init(Sstar_cmd_hash_head, SSTAR_CMD_HASHENTRIES);

	for (index = 0; index < ARRAY_SIZE(cmd_code_buff); index++) {
		unsigned int hash_index =
			Sstar_hash_index(cmd_code_buff[index].label,
					 strlen(cmd_code_buff[index].label),
					 SSTAR_CMD_HASHBITS);
		BUG_ON(strlen(cmd_code_buff[index].label) >
		       SSTAR_CMD_MAX_STRING_LEN);
		hlist_add_head(&cmd_code_buff[index].hnode,
			       &Sstar_cmd_hash_head[hash_index]);
		cmd_code_buff[index].echo_ready = false;
		Sstar_store_cmd_deinit_show(&cmd_code_buff[index].echo);
		Sstar_printk_debug("%s:[%s]->[%d]\n", __func__,
				   cmd_code_buff[index].label, hash_index);
	}

	backup_show = Sstar_kzalloc(SSTAR_SHOW_MSG_MAX_SIZE, GFP_KERNEL);
	WARN_ON(backup_show == NULL);
	backup_show_size = 0;
}

static void Sstar_module_cmd_exit(void)
{
	while (!list_empty(&Sstar_cmd_echo_ready_list)) {
		struct Sstar_store_cmd_code *cmd_code =
			list_first_entry(&Sstar_cmd_echo_ready_list,
					 struct Sstar_store_cmd_code, lhead);
		if (cmd_code->show_cmd)
			cmd_code->show_cmd(Sstar_hw_priv_dereference(),
					   cmd_code, NULL, false);
		Sstar_store_cmd_echo_done(cmd_code);
		Sstar_store_cmd_deinit_show(&cmd_code->echo);
	}

	if (backup_show) {
		Sstar_kfree(backup_show);
	}
	backup_show = NULL;
	backup_show_size = 0;
}
static void Sstar_module_firmware_caps_show(struct Sstar_module_show *show_buff,
					    struct Sstar_common *hw_priv)
{
#define WSM_CAP(cap) !!(hw_priv->wsm_caps.firmwareCap & cap)

	Sstar_module_show_put(show_buff, LIGHT "Firmare Cap:" NORMAL ENTER);
	Sstar_module_show_put(show_buff, "PRIVATE_IE  [%d]\n",
			      WSM_CAP(CAPABILITIES_SSTAR_PRIVATE_IE));
	Sstar_module_show_put(show_buff, "NVR_IPC     [%d]\n",
			      WSM_CAP(CAPABILITIES_NVR_IPC));
	Sstar_module_show_put(show_buff, "NO_CONFIRM  [%d]\n",
			      WSM_CAP(CAPABILITIES_NO_CONFIRM));
	Sstar_module_show_put(show_buff, "SDIO_PATCH  [%d]\n",
			      WSM_CAP(CAPABILITIES_SDIO_PATCH));
	Sstar_module_show_put(show_buff, "NO_BACKOFF  [%d]\n",
			      WSM_CAP(CAPABILITIES_NO_BACKOFF));
	Sstar_module_show_put(show_buff, "CFO         [%d]\n",
			      WSM_CAP(CAPABILITIES_CFO));
	Sstar_module_show_put(show_buff, "AGC         [%d]\n",
			      WSM_CAP(CAPABILITIES_AGC));
	Sstar_module_show_put(show_buff, "TXCAL       [%d]\n",
			      WSM_CAP(CAPABILITIES_TXCAL));
	Sstar_module_show_put(show_buff, "CTS_BUG     [%d]\n",
			      WSM_CAP(CAPABILITIES_CTS_BUG));
	Sstar_module_show_put(show_buff, "MONITOR     [%d]\n",
			      WSM_CAP(CAPABILITIES_MONITOR));
	Sstar_module_show_put(show_buff, "CUSTOM      [%d]\n",
			      WSM_CAP(CAPABILITIES_CUSTOM));
	Sstar_module_show_put(show_buff, "SMARTCONFIG [%d]\n",
			      WSM_CAP(CAPABILITIES_SMARTCONFIG));
	Sstar_module_show_put(show_buff, "ETF         [%d]\n",
			      WSM_CAP(CAPABILITIES_ETF));
	Sstar_module_show_put(show_buff, "LMAC_RATECTL[%d]\n",
			      WSM_CAP(CAPABILITIES_LMAC_RATECTL));
	Sstar_module_show_put(show_buff, "LMAC_TPC    [%d]\n",
			      WSM_CAP(CAPABILITIES_LMAC_TPC));
	Sstar_module_show_put(show_buff, "LMAC_TEMPC  [%d]\n",
			      WSM_CAP(CAPABILITIES_LMAC_TEMPC));
	Sstar_module_show_put(show_buff, "USE_IPC     [%d]\n",
			      WSM_CAP(CAPABILITIES_USE_IPC));
	Sstar_module_show_put(show_buff, "OUTER_PA    [%d]\n",
			      WSM_CAP(CAPABILITIES_OUTER_PA));
	Sstar_module_show_put(show_buff, "HW_CHECKSUM [%d]\n",
			      WSM_CAP(CAPABILITIES_HW_CHECKSUM));
	Sstar_module_show_put(show_buff, "MULTI_RX    [%d]\n",
			      WSM_CAP(CAPABILITIES_SINGLE_CHANNEL_MULTI_RX));
	Sstar_module_show_put(show_buff, "USB_RECOVERY_BUG      [%d]\n",
			      WSM_CAP(CAPABILITIES_USB_RECOVERY_BUG));
	Sstar_module_show_put(show_buff, "POWER_CONSUMPTION     [%d]\n",
			      WSM_CAP(CAPABILITIES_POWER_CONSUMPTION));
	Sstar_module_show_put(show_buff, "RSSI_DECIDE_TXPOWER   [%d]\n",
			      WSM_CAP(CAPABILITIES_RSSI_DECIDE_TXPOWER));
	Sstar_module_show_put(show_buff, "RTS_LONG_DUR          [%d]\n",
			      WSM_CAP(CAPABILITIES_RTS_LONG_DURATION));
	Sstar_module_show_put(show_buff, "TX_CFO_PPM_CORRECTION [%d]\n",
			      WSM_CAP(CAPABILITIES_TX_CFO_PPM_CORRECTION));
	Sstar_module_show_put(show_buff, "NOISE_SET_DCXO        [%d]\n",
			      WSM_CAP(CAPABILITIES_NOISE_SET_DCXO));
}

static void Sstar_module_driver_caps_show(struct Sstar_module_show *show_buff,
					  struct Sstar_common *hw_priv)
{
	Sstar_module_show_put(show_buff, LIGHT "Driver Cap:" NORMAL ENTER);
#ifdef USB_BUS
	Sstar_module_show_put(show_buff, "HIF_TYPE    [%s]\n", "USB");
#endif
#ifdef SDIO_BUS
	Sstar_module_show_put(show_buff, "HIF_TYPE    [%s]\n", "SDIO");
#endif
#ifdef SPI_BUS
	Sstar_module_show_put(show_buff, "HIF_TYPE    [%s]\n", "SPI");
#endif
#ifdef SSTAR_NOT_SUPPORT_40M_CHW
	Sstar_module_show_put(show_buff, "HW_CHW      [%s]\n", "20M");
#else
	Sstar_module_show_put(show_buff, "HW_CHW      [%s]\n", "40M");
#endif
#ifdef CONFIG_SSTAR_5G_PRETEND_2G
	Sstar_module_show_put(show_buff, "BAND_SUPPORT[%s]\n", "5G and 2G");
#else
	Sstar_module_show_put(show_buff, "BAND_SUPPORT[%s]\n", "only 2G");
#endif
}
static ssize_t Sstar_module_show_system_info(struct kobject *kobj,
					     struct kobj_attribute *attr,
					     char *buf)
{
	struct Sstar_module_show sys_show;
	struct Sstar_common *hw_priv = NULL;

	Sstar_show_init(&sys_show, buf);

	Sstar_module_muxlock();
	if (Sstar_hw_priv_dereference() == NULL) {
		Sstar_module_show_put(
			&sys_show, "system info not exit,please plug chip\n");
		goto exit;
	}
	hw_priv = Sstar_hw_priv_dereference();
	Sstar_module_show_put(&sys_show, LIGHT "DriverVer   [%d]" NORMAL ENTER,
			      DRIVER_VER);
	Sstar_module_show_put(&sys_show, LIGHT "FirmwareVer [%d]" NORMAL ENTER,
			      hw_priv->wsm_caps.firmwareVersion);
	Sstar_module_firmware_caps_show(&sys_show, hw_priv);
	Sstar_module_driver_caps_show(&sys_show, hw_priv);
exit:
	Sstar_module_muxunlock();

	return sys_show.show_count;
}
static ssize_t Sstar_module_show_backup_info(struct kobject *kobj,
					     struct kobj_attribute *attr,
					     char *buf)
{
	ssize_t size = 0;

	Sstar_modulefs_lock();
	Sstar_module_muxlock();
	if (backup_show && backup_show_size) {
		size = backup_show_size;
		if (WARN_ON(size > SSTAR_SHOW_MSG_MAX_SIZE))
			size = SSTAR_SHOW_MSG_MAX_SIZE;
		memcpy(buf, backup_show, SSTAR_SHOW_MSG_MAX_SIZE);
	}
	Sstar_module_muxunlock();
	Sstar_modulefs_unlock();

	return size;
}

static ssize_t Sstar_module_show_get_efuse(struct kobject *kobj,
					   struct kobj_attribute *attr,
					   char *buf)
{
	struct Sstar_module_show sys_show;
	struct Sstar_common *hw_priv = NULL;
	struct efuse_headr efuse_data;

	Sstar_show_init(&sys_show, buf);
	memset(&efuse_data, 0, sizeof(struct efuse_headr));
	Sstar_module_muxlock();
	if (Sstar_hw_priv_dereference() == NULL) {
		Sstar_module_show_put(
			&sys_show,
			"Sstar_module_show_get_efuse:system info not exit,please plug chip\n");
		goto exit;
	}
	hw_priv = Sstar_hw_priv_dereference();
	if (wsm_get_efuse_data(hw_priv, &efuse_data, sizeof(efuse_data)) == 0) {
		if (memcmp(&hw_priv->efuse, &efuse_data, sizeof(efuse_data)) !=
		    0) {
			Sstar_printk_err(
				"Sstar_module_show_get_efuse:efsue not same ! \n");
		}
	} else {
		Sstar_printk_err("get efsue error! \n");
		goto exit;
	}
	Sstar_module_show_put(&sys_show, "dcxo[%d]\n", efuse_data.dcxo_trim);
	Sstar_module_show_put(&sys_show, "gain1[%d]\n", efuse_data.delta_gain1);
	Sstar_module_show_put(&sys_show, "gain2[%d]\n", efuse_data.delta_gain2);
	Sstar_module_show_put(&sys_show, "gain3[%d]\n", efuse_data.delta_gain3);
	Sstar_module_show_put(&sys_show, "mac[%02x:%02x:%02x:%02x:%02x:%02x]\n",
			      efuse_data.mac[0], efuse_data.mac[1],
			      efuse_data.mac[2], efuse_data.mac[3],
			      efuse_data.mac[4], efuse_data.mac[5]);

exit:
	Sstar_module_muxunlock();
	return sys_show.show_count;
}

static ssize_t Sstar_module_show_first_efuse(struct kobject *kobj,
					     struct kobj_attribute *attr,
					     char *buf)
{
	struct Sstar_module_show sys_show;
	struct Sstar_common *hw_priv = NULL;
	struct efuse_headr efuse_data;

	Sstar_show_init(&sys_show, buf);
	memset(&efuse_data, 0, sizeof(struct efuse_headr));
	Sstar_module_muxlock();
	if (Sstar_hw_priv_dereference() == NULL) {
		Sstar_module_show_put(
			&sys_show,
			"Sstar_module_show_get_efuse:system info not exit,please plug chip\n");
		goto exit;
	}
	hw_priv = Sstar_hw_priv_dereference();
	if (wsm_get_efuse_first_data(hw_priv, &efuse_data,
				     sizeof(efuse_data)) != 0) {
		Sstar_printk_err("get first efsue error! \n");
		goto exit;
	}
	Sstar_module_show_put(&sys_show, "dcxo[%d]\n", efuse_data.dcxo_trim);
	Sstar_module_show_put(&sys_show, "gain1[%d]\n", efuse_data.delta_gain1);
	Sstar_module_show_put(&sys_show, "gain2[%d]\n", efuse_data.delta_gain2);
	Sstar_module_show_put(&sys_show, "gain3[%d]\n", efuse_data.delta_gain3);
	Sstar_module_show_put(&sys_show, "mac[%02x:%02x:%02x:%02x:%02x:%02x]\n",
			      efuse_data.mac[0], efuse_data.mac[1],
			      efuse_data.mac[2], efuse_data.mac[3],
			      efuse_data.mac[4], efuse_data.mac[5]);

exit:
	Sstar_module_muxunlock();
	return sys_show.show_count;
}

static ssize_t Sstar_module_show_remain_efuse(struct kobject *kobj,
					      struct kobj_attribute *attr,
					      char *buf)
{
	struct Sstar_module_show sys_show;
	struct Sstar_common *hw_priv = NULL;
	int remainBit = 0;

	Sstar_show_init(&sys_show, buf);

	Sstar_module_muxlock();
	if (Sstar_hw_priv_dereference() == NULL) {
		Sstar_module_show_put(
			&sys_show,
			"Sstar_module_show_get_efuse:system info not exit,please plug chip\n");
		goto exit;
	}
	hw_priv = Sstar_hw_priv_dereference();
	if (wsm_get_efuse_remain_bit(hw_priv, &remainBit, sizeof(int)) != 0) {
		Sstar_printk_err("get  efsue reamin space error! \n");
		goto exit;
	}

	Sstar_module_show_put(&sys_show, "reamin Efuse[%d bit]\n", remainBit);

exit:
	Sstar_module_muxunlock();
	return sys_show.show_count;
}

static ssize_t Sstar_module_show_cfg_power(struct kobject *kobj,
					   struct kobj_attribute *attr,
					   char *buf)
{
	struct Sstar_module_show sys_show;
	struct Sstar_common *hw_priv = NULL;
	struct cfg_txpower_t configured_txpower;
	int i, ret;
	const char *ratebuf[11] = {
		"b_1M_2M",     "b_5_5M_11M",	"g_6M_n_6_5M", "g_9M",
		"g_12M_n_13M", "g_18M_n_19_5M", "g_24M_n_26M", "g_36M_n_39M",
		"g_48M_n_52M", "g_54M_n_58_5M", "n_65M"
	};

	memset(&configured_txpower, 0, sizeof(configured_txpower));

	Sstar_show_init(&sys_show, buf);

	Sstar_module_muxlock();
	if (Sstar_hw_priv_dereference() == NULL) {
		Sstar_module_show_put(
			&sys_show,
			"Sstar_module_show_get_efuse:system info not exit,please plug chip\n");
		goto exit;
	}
	hw_priv = Sstar_hw_priv_dereference();
#if defined(CONFIG_TXPOWER_DCXO_VALUE) || defined(CONFIG_RATE_TXPOWER)

	if ((ret = wsm_get_cfg_txpower(hw_priv, (void *)&configured_txpower,
				       sizeof(configured_txpower))) == 0) {
		for (i = 0; i < sizeof(configured_txpower.set_txpwr_delta_gain);
		     i++)
			Sstar_module_show_put(
				&sys_show, "delta_gain%d:%d\n", i + 1,
				configured_txpower.set_txpwr_delta_gain[i]);
		for (i = 0;
		     i < sizeof(configured_txpower.set_b_txpwr_delta_gain); i++)
			Sstar_module_show_put(
				&sys_show, "b_gain%d:%d\n", i + 1,
				configured_txpower.set_b_txpwr_delta_gain[i]);
		for (i = 0;
		     i < sizeof(configured_txpower.set_gn_txpwr_delta_gain);
		     i++)
			Sstar_module_show_put(
				&sys_show, "gn_gain%d:%d\n", i + 1,
				configured_txpower.set_gn_txpwr_delta_gain[i]);
		Sstar_module_show_put(&sys_show, "\n\n");
		for (i = 0;
		     i < sizeof(configured_txpower.different_rate_txpower_mode);
		     i++)
			Sstar_module_show_put(
				&sys_show, "%s:%d\n", ratebuf[i],
				configured_txpower
					.different_rate_txpower_mode[i]);
		for (i = 0;
		     i <
		     sizeof(configured_txpower.different_rate_txpower_mode_40M);
		     i++)
			Sstar_module_show_put(
				&sys_show, "%s_40M:%d\n", ratebuf[i],
				configured_txpower
					.different_rate_txpower_mode_40M[i]);
	} else {
		Sstar_printk_err("read configured tx power failed\n");
	}
#else
	Sstar_printk_err(
		"undefine CONFIG_RATE_TXPOWER or CONFIG_TXPOWER_DCXO_VALUE,so Not support\n");
#endif

exit:
	Sstar_module_muxunlock();
	return sys_show.show_count;
}

static struct hlist_head *Sstar_printk_hash_list(const char *string,
						 unsigned int len)
{
	return Sstar_hash_list(string, len, Sstar_printk_hash_head,
			       SSTAR_PRINTK_HASHBITS);
}
static struct Sstar_printk_mask_table_s *
Sstar_printk_find_mask_table(const char *pos, unsigned int pos_len)
{
	unsigned int string_len = 0;
	struct hlist_head *printk_hlist = NULL;
	struct Sstar_printk_mask_table_s *printk_node;
	struct hlist_node *node;

	if (SSTAR_PRINTK_MAX_STRING_LEN < pos_len) {
		Sstar_printk_always("%s:[%d] pos_len err\n", __func__, pos_len);
		return NULL;
	}

	string_len = Sstar_hash_string_len(pos, pos_len);
	if (string_len == 0) {
		Sstar_printk_always("%s:[%s] string_len err\n", __func__, pos);
		return NULL;
	}

	printk_hlist = Sstar_printk_hash_list(pos, string_len);

	hlist_for_each (node, printk_hlist) {
		printk_node = hlist_entry(
			node, struct Sstar_printk_mask_table_s, string_hnode);
		if ((strlen(printk_node->mask_string) == string_len) &&
		    (!strncmp(pos, printk_node->mask_string, string_len))) {
			return printk_node;
		}
	}
	Sstar_printk_always("%s:[%s] can not fund\n", __func__, pos);
	return NULL;
}
static ssize_t Sstar_module_printk_mask_show(struct kobject *kobj,
					     struct kobj_attribute *attr,
					     char *buf)
{
	struct Sstar_module_show sys_show;
	u8 log_index = 0;
	Sstar_show_init(&sys_show, buf);

	Sstar_module_show_put(&sys_show,
			      LIGHT "Debug Level Help--->" NORMAL ENTER);
	for (log_index = 0; log_index < ARRAY_SIZE(printk_mask_table);
	     log_index++) {
		Sstar_module_show_put(&sys_show, LIGHT "[%s]" NORMAL ":%s\n",
				      printk_mask_table[log_index].mask_string,
				      printk_mask_table[log_index].mask_info);
	}
	Sstar_module_show_put(&sys_show,
			      LIGHT "Current Log Level--->" NORMAL ENTER);
	for (log_index = 0; log_index < ARRAY_SIZE(printk_mask_table);
	     log_index++) {
		if ((printk_mask_table[log_index].mask_val) &
		    Sstar_printk_mask) {
			Sstar_module_show_put(
				&sys_show, "[%s]\n",
				printk_mask_table[log_index].mask_string);
		}
	}
	return sys_show.show_count;
}
static bool Sstar_str_usefull(const char *target)
{
	return (target[0] == SSTAR_UNUSED ? false : true);
}
static bool Sstar_module_printk_mask_parase(const char *mask_str,
					    ssize_t msg_len)
{
	const char *pos = mask_str;
	const char *nospace_pos = mask_str;
	const char *pos_end = mask_str + msg_len;
	const char *val_pos;
	ssize_t usefull_len = msg_len;
	ssize_t skip_len = 0;
	const struct Sstar_printk_mask_table_s *table = NULL;

	while (pos < pos_end) {
		nospace_pos = Sstar_skip_space(pos, pos_end - pos);

		if (nospace_pos == NULL) {
			Sstar_printk_always("%s nospace_pos == NULL\n",
					    __func__);
			return pos == mask_str ? false : true;
		}

		pos = nospace_pos;
		usefull_len = pos_end - pos;

		if (pos[0] == SSTAR_TAIL) {
			return pos == mask_str ? false : true;
		} else if ((pos[0] == SSTAR_LINEF) || (pos[0] == SSTAR_ENTER)) {
			return pos == mask_str ? false : true;
		} else if ((!strncmp(pos, LOG_DEFAULT, strlen(LOG_DEFAULT))) &&
			   (!Sstar_string_invalid(pos[strlen(LOG_DEFAULT)]))) {
			Sstar_printk_mask = SSTAR_PRINTK_DEFAULT_MASK;
			skip_len = strlen(LOG_DEFAULT);
		} else if ((!strncmp(pos, LOG_CLEAR, strlen(LOG_CLEAR))) &&
			   (!Sstar_string_invalid(pos[strlen(LOG_CLEAR)]))) {
			skip_len = strlen(LOG_CLEAR);
			Sstar_printk_mask = SSTAR_PRINTK_CLEAR;
		} else if ((!strncmp(pos, LOG_ALL, strlen(LOG_ALL))) &&
			   (!Sstar_string_invalid(pos[strlen(LOG_ALL)]))) {
			skip_len = strlen(LOG_ALL);
			Sstar_printk_mask = SSTAR_PRINTK_ALL;
		} else if (memchr(pos, SSTAR_EQUAL, pos_end - pos) == NULL) {
			return false;
		} else {
			u8 index = 0;
			/*
			*find '='
			*/
			val_pos = memchr(pos, SSTAR_EQUAL, pos_end - pos);
			if ((val_pos == NULL) || (pos == val_pos) ||
			    (pos_end == val_pos)) {
				Sstar_printk_always("can not find = %s:%s\n",
						    pos, val_pos);
				return false;
			}

			table = Sstar_printk_find_mask_table(pos,
							     val_pos - pos);

			if (table == NULL) {
				Sstar_printk_always(
					"can not find hash tabel:%s\n", pos);
				return false;
			}

			val_pos++;
			val_pos = Sstar_skip_space(
				val_pos, usefull_len - (val_pos - pos));
			if (val_pos == NULL) {
				Sstar_printk_always("must be ON or OFF:%s\n",
						    val_pos);
				return false;
			}
			/*
			*len > strlen("ON")
			*/
			if (pos_end - val_pos < strlen(LOG_ON)) {
				Sstar_printk_always("%s len is not enough %s\n",
						    __func__, val_pos);
				return false;
			}
			/*
			*is on or off
			*/
			for (index = 0; index < ARRAY_SIZE(option_table);
			     index++) {
				const char *option_pos = NULL;
				u8 option_len = 0;
				if ((option_pos = strstr(
					     val_pos, option_table[index]
							      .opton_string)) ==
				    NULL) {
					continue;
				}

				if (option_pos != val_pos) {
					Sstar_printk_always(
						"option_pos(%s),val_pos(%s)\n",
						option_pos, val_pos);
					continue;
				}

				option_len += strlen(
					option_table[index].opton_string);

				/*
				*between cmd is space
				*/
				if (Sstar_string_invalid(val_pos[option_len])) {
					Sstar_printk_always(
						"option format err [%s][%s]\n",
						pos, val_pos);
					continue;
				}
				Sstar_printk_always(
					"[%s=%s]--->[%s=%s]\n",
					table->mask_string,
					Sstar_printk_mask & table->mask_val ?
						      LOG_ON :
						      LOG_OFF,
					table->mask_string,
					option_table[index].opton_string);
				val_pos += option_len;
				if (option_table[index].option_val) {
					Sstar_printk_mask |= table->mask_val;
				} else {
					Sstar_printk_mask &= ~table->mask_val;
				}
				break;
			}

			if (index == ARRAY_SIZE(option_table)) {
				Sstar_printk_always(
					"option not found [%s][%s]\n", pos,
					val_pos);
				return false;
			}
			skip_len = val_pos - pos;
		}

		pos += skip_len;
	}
	return pos == mask_str ? false : true;
}
//#include <stdlib.h>

static ssize_t Sstar_module_set_printk_level_store(struct kobject *kobj,
						   struct kobj_attribute *attr,
						   const char *buf, size_t n)
{
	//	int printk_level;
	Sstar_printk_always("[%s]:%s\n", __func__, buf);
	//	printk_level = atoi(buf);
	Sstar_printk_always("printk_level = %d \n", buf[0]);
	switch (buf[0]) {
	case 0x30: {
		Sstar_printk_mask = SSTAR_PRINTK_CLEAR;
		Sstar_printk_always("SSTAR_PRINTK_CLEAR\n");
	} break;
	case 0x31: {
		Sstar_printk_mask = SSTAR_PRINTK_LEVEL1;
		Sstar_printk_always("SSTAR_PRINTK_LEVEL==>1\n");
	} break;
	case 0x32: {
		Sstar_printk_mask = SSTAR_PRINTK_LEVEL2;
		Sstar_printk_always("SSTAR_PRINTK_LEVEL==>2\n");
	} break;
	case 0x33: {
		Sstar_printk_mask = SSTAR_PRINTK_LEVEL3;
		Sstar_printk_always("SSTAR_PRINTK_LEVEL==>3\n");
	} break;
	case 0x34:
	case 0x35: {
		Sstar_printk_mask = SSTAR_PRINTK_ALL;
		Sstar_printk_always("SSTAR_PRINTK_ALL\n");
	} break;
	default: {
		Sstar_printk_always("SSTAR_PRINTK_NO_CHANGE\n");
	} break;
	}
	return -EINVAL;
}

static ssize_t Sstar_module_printk_mask_store(struct kobject *kobj,
					      struct kobj_attribute *attr,
					      const char *buf, size_t n)
{
	const char *p_enter = NULL;
	const char *p = NULL;
	const char *nospace_pos = NULL;
	const char *pos = buf;
	const char *pos_end = buf + n;
	ssize_t line_len = 0;
	ssize_t msg_len;
	ssize_t len_left = n;

	Sstar_printk_always("[%s]:%s\n", __func__, buf);
	do {
		if (pos >= pos_end) {
			Sstar_printk_always("%s end 1\n", __func__);
			break;
		}
		pos = Sstar_skip_space(pos, len_left);
		if (pos == NULL) {
			Sstar_printk_always("pos (%s) err\n", buf);
			break;
		}
		if (pos[0] == SSTAR_TAIL) {
			break;
		}
		/*
		*find line of the file
		*linux:\n
		*windows:\r\n
		*mac \r
		*/
		p_enter = memchr(pos, SSTAR_LINEF, len_left);
		if (p_enter == NULL) {
			p_enter = memchr(pos, SSTAR_ENTER, len_left);
		}
		if (p_enter == NULL) {
			Sstar_printk_always("%s:can not fine aline\n",
					    __func__);
		}
		line_len = p_enter ? p_enter - pos : len_left;

		if (line_len <= 0) {
			Sstar_printk_always("%s end 2\n", __func__);
			break;
		}

		p = pos;
		/*
		*pos pointer next line
		*/
		if (p_enter == NULL) {
			pos += line_len;
		} else {
			pos += (line_len + 1);
		}

		if (pos > pos_end) {
			Sstar_printk_always("%s pos > pos_end\n", __func__);
			break;
		}

		len_left = pos_end - pos;

		msg_len = line_len;
		/*
		*skip header space
		*/
		nospace_pos = Sstar_skip_space(p, msg_len);

		if (nospace_pos == NULL) {
			Sstar_printk_always("%s nospace_pos == NULL\n",
					    __func__);
			continue;
		}

		msg_len -= nospace_pos - p;

		p = nospace_pos;
		/*
		*'#' means unusefull
		*/
		if (Sstar_str_usefull(p) == false) {
			Sstar_printk_always("%s p is not usefull\n", __func__);
			continue;
		}
		if (Sstar_module_printk_mask_parase(p, msg_len) == false) {
			Sstar_printk_always("%s parase err\n", __func__);
			return -EINVAL;
		}
	} while (1);
	return pos != pos_end ? n : -EINVAL;
}
static void Sstar_module_printk_init(void)
{
	unsigned int index = 0;

	Sstar_common_hash_list_init(Sstar_printk_hash_head,
				    SSTAR_PRINTK_HASHENTRIES);

	for (index = 0; index < ARRAY_SIZE(printk_mask_table); index++) {
		unsigned int hash_index = Sstar_hash_index(
			printk_mask_table[index].mask_string,
			strlen(printk_mask_table[index].mask_string),
			SSTAR_PRINTK_HASHBITS);
		BUG_ON(strlen(printk_mask_table[index].mask_string) >
		       SSTAR_PRINTK_MAX_STRING_LEN);
		Sstar_printk_debug("%s:[%s]->[%d]\n", __func__,
				   printk_mask_table[index].mask_string,
				   hash_index);
		hlist_add_head(&printk_mask_table[index].string_hnode,
			       &Sstar_printk_hash_head[hash_index]);
	}
}

static struct kobject *Sstar_module_kobj = NULL;

static struct kobj_attribute Sstar_module_common_attr =
	__ATTR(Sstar_cmd, 0644, Sstar_module_cmd_show, Sstar_module_cmd_store);
static struct kobj_attribute Sstar_module_sysinfo_attr =
	__ATTR(Sstar_sys, 0444, Sstar_module_show_system_info, NULL);
static struct kobj_attribute Sstar_module_common_backup_attr =
	__ATTR(Sstar_sys_backup, 0444, Sstar_module_show_backup_info, NULL);
static struct kobj_attribute Sstar_module_getefuse_attr =
	__ATTR(Sstar_efuse, 0444, Sstar_module_show_get_efuse, NULL);
static struct kobj_attribute Sstar_module_getfirsyefuse_attr =
	__ATTR(Sstar_first_efuse, 0444, Sstar_module_show_first_efuse, NULL);
static struct kobj_attribute Sstar_module_remainefuse_attr = __ATTR(
	Sstar_remainEfuseSpace, 0444, Sstar_module_show_remain_efuse, NULL);
static struct kobj_attribute Sstar_module_get_rate_power =
	__ATTR(Sstar_get_cfg_power, 0444, Sstar_module_show_cfg_power, NULL);
static struct kobj_attribute Sstar_module_set_print_level = __ATTR(
	Sstar_set_print_level, 0644, NULL, Sstar_module_set_printk_level_store);

static struct kobj_attribute Sstar_module_printk_mask =
	__ATTR(Sstar_printk_mask, 0644, Sstar_module_printk_mask_show,
	       Sstar_module_printk_mask_store);

static struct attribute *Sstar_module_attribute_group[] = {
	&Sstar_module_common_attr.attr,
	&Sstar_module_common_backup_attr.attr,
	&Sstar_module_sysinfo_attr.attr,
	&Sstar_module_printk_mask.attr,
	&Sstar_module_getefuse_attr.attr,
	&Sstar_module_getfirsyefuse_attr.attr,
	&Sstar_module_remainefuse_attr.attr,
	&Sstar_module_get_rate_power.attr,
	&Sstar_module_set_print_level.attr,
	NULL,
};
static struct attribute_group Sstar_module_attr_group = {
	.attrs = Sstar_module_attribute_group,
};

int Sstar_module_attribute_init(void)
{
	int error;
	struct kobject *parent = Sstar_module_parent;

	Sstar_module_kobj = kobject_create_and_add("Sstarfs", parent);
	if (!Sstar_module_kobj) {
		return -EINVAL;
	}
	error = sysfs_create_group(Sstar_module_kobj, &Sstar_module_attr_group);
	if (error)
		kobject_put(Sstar_module_kobj);
	Sstar_modulefs_lock_int();
	Sstar_module_printk_init();
	Sstar_module_cmd_init();
	return error;
}
void Sstar_module_attribute_exit(void)
{
	if (Sstar_module_kobj == NULL)
		return;
	sysfs_remove_group(Sstar_module_kobj, &Sstar_module_attr_group);
	kobject_put(Sstar_module_kobj);
	Sstar_modulefs_lock();
	Sstar_module_cmd_exit();
	Sstar_modulefs_unlock();
	Sstar_modulefs_lock_release();
}
#endif
