#include "public_db.h"
#include "base/encrypt/encrypt_impl.h"
#include "shared/tool.h"

namespace
{
#define LOGIN_DATA_FILE		"app_login_data.db"
static std::vector<UTF8String> kCreateDBSQLs;
}

PublicDB::PublicDB()
{
    static bool sqls_created = false;
    if (!sqls_created)
    {
		kCreateDBSQLs.push_back("CREATE TABLE IF NOT EXISTS logindata(uid TEXT PRIMARY KEY, \
								 								                                   name TEXT, password TEXT, type INTEGER, status INTEGER, remember_user INTEGER, remember_psw INTEGER, autologin INTEGER, user_header TEXT)");
		kCreateDBSQLs.push_back("CREATE INDEX IF NOT EXISTS logindatauidindex ON logindata(uid)");

		kCreateDBSQLs.push_back("CREATE TABLE IF NOT EXISTS proxysettingdata(proxytype INTEGER PRIMARY KEY, \
								 address TEXT, port TEXT,name TEXT, password TEXT, domain TEXT,valid INTEGER)");
		kCreateDBSQLs.push_back("CREATE INDEX IF NOT EXISTS proxydataindex ON proxysettingdata(proxytype)");

		kCreateDBSQLs.push_back("CREATE TABLE IF NOT EXISTS config_info(key TEXT PRIMARY KEY, value TEXT)");

		//kCreateDBSQLs.push_back("DROP TABLE customImage;)");
		kCreateDBSQLs.push_back("CREATE TABLE IF NOT EXISTS customImage(imgId TEXT PRIMARY KEY, uid TEXT, OriginalPath TEXT, thumbnailPath TEXT)");
		
		kCreateDBSQLs.push_back("CREATE TABLE IF NOT EXISTS client_config_info(id TEXT PRIMARY KEY, uid TEXT, tid TEXT, value TEXT, date TEXT)");

		kCreateDBSQLs.push_back("CREATE TABLE IF NOT EXISTS item_top_info(id TEXT PRIMARY KEY, uid TEXT, accid TEXT, value TEXT)");

		/*kCreateDBSQLs.push_back("CREATE TABLE IF NOT EXISTS friend_group(id TEXT,macct TEXT,groupname TEXT,friendacct TEXT)");*/

		kCreateDBSQLs.push_back("CREATE TABLE IF NOT EXISTS item_value(skey TEXT,svalue TEXT)");
		sqls_created = true;
    }
	aes_key_ = "12345500072bf3390c79f01004dabcde";//32位
	db_encrypt_key_ = "1234560247a0619f07548fb1b8abcedf";//注意：只支持最多32个字符的加密密钥！

	this->Load();
}

PublicDB::~PublicDB()
{
	this->Close();
}

ndb::SQLiteDB& PublicDB::GetSQLiteDB()
{
	return db_;
}
bool PublicDB::Load()
{
	return CreateDBFile();
}

void PublicDB::Close()
{
    db_.Close();
}

bool PublicDB::WriteCustomData(CustomData &data)
{
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	db_.Query(stmt,
		"INSERT INTO customImage(imgId, uid, OriginalPath, thumbnailPath) VALUES (?, ?, ?, ?);");
	stmt.BindText(1, data.img_id_.c_str(), (int)data.img_id_.size());
	stmt.BindText(2, data.user_id_.c_str(), (int)data.user_id_.size());
	stmt.BindText(3, data.OriginalPath.c_str(), (int)data.OriginalPath.size());
	stmt.BindText(4, data.thumbnailPath.c_str(), (int)data.thumbnailPath.size());

	int32_t result = stmt.NextRow();
	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (false == no_error)
	{
		QLOG_APP(L"error: insert custom image data for uid : {0}, reason : {1}") << data.user_id_ << result;
	}
	return no_error;
}

bool PublicDB::DeleteCustomData(CustomData &data)
{
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	db_.Query(stmt,"DELETE FROM customImage WHERE uid=? AND imgId=?;");

	stmt.BindText(1, data.user_id_.c_str(), (int)data.user_id_.size());
	stmt.BindText(2, data.img_id_.c_str(), (int)data.img_id_.size());
	//std::string strSql = "DELETE FROM customImage WHERE uid = " + data.user_id_ + " AND imgId = " + data.img_id_;
	//db_.Query(stmt, strSql.data());

	int32_t result = stmt.NextRow();
	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (false == no_error)
	{
		QLOG_APP(L"error: delete custom image data for uid : {0}, reason : {1}") << data.user_id_ << result;
	}
	return no_error;
}

std::string PublicDB::GetImgPathFromImgId(UTF8String user_id_, UTF8String img_id_)
{
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	//std::string strSql = "SELETE * FROM customImage WHERE uid = " + user_id_ + " AND imgId = " + img_id_;
	db_.Query(stmt, "SELECT * FROM customImage WHERE uid=? AND imgId=?");
	stmt.BindText(1, user_id_.c_str(), (int)user_id_.size());
	stmt.BindText(2, img_id_.c_str(), (int)img_id_.size());
	//db_.Query(stmt, strSql.data());

	int32_t result = stmt.NextRow();
	while (result == SQLITE_ROW)
	{
		std::string originalPath = stmt.GetTextField(2);
		if (!originalPath.empty())
		{
			return originalPath;
		}
	}
	return "";
}

bool PublicDB::WriteForcePushData(std::string uid, std::string tid, std::string date, std::string count)
{
	nbase::NAutoLock auto_lock(&lock_);
	//bool result = false;
	ndb::SQLiteStatement stmt;
	db_.Query(stmt, "SELECT * FROM client_config_info WHERE uid=? AND tid=?");
	stmt.BindText(1, uid.c_str(), (int)uid.size());
	stmt.BindText(2, tid.c_str(), (int)tid.size());
	uint32_t db_reslut = stmt.NextRow();

	if (db_reslut == SQLITE_OK || db_reslut == SQLITE_ROW)
	{
		//GetLoginDataFromStatement(stmt, data);
		//result = true;
		
		//nbase::NAutoLock auto_lock(&lock_);
		UTF8String query_sql;
		nbase::StringPrintf(query_sql, "UPDATE OR ROLLBACK client_config_info SET date = '%s', value = '%s' \
									   									   									   WHERE uid = '%s' AND tid = '%s'",
																											   date.c_str(),
																											   count.c_str(),
																											   uid.c_str(),
																											   tid.c_str());
		int32_t result = db_.Query(query_sql.c_str());
		bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
		if (!no_error)
		{
			QLOG_APP(L"Error: Set client_config_info count For uid: {0},  tid : {1}") << uid << tid;
		}

		return no_error;

	}
	
	ndb::SQLiteStatement stmt1;
	db_.Query(stmt1,
		"INSERT INTO client_config_info(uid, tid, date, value) VALUES (?, ?, ?, ?);");

	stmt1.BindText(1, uid.c_str(), (int)uid.size());
	stmt1.BindText(2, tid.c_str(), (int)tid.size());
	stmt1.BindText(3, date.c_str(), (int)date.size());
	stmt1.BindText(4, count.c_str(), (int)count.size());

	int32_t result = stmt1.NextRow();
	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (false == no_error)
	{
		QLOG_APP(L"error: insert client_config_info data for uid : {0}, tid : {1}") << uid << tid;
	}
	return no_error;
}

int PublicDB::ReadForcePushOneData(const std::string userId, const std::string tid, std::string date)
{
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	db_.Query(stmt, "SELECT value FROM client_config_info WHERE uid=? AND tid=? AND date=?");
	stmt.BindText(1, userId.c_str(), (int)userId.size());
	stmt.BindText(2, tid.c_str(), (int)tid.size());
	stmt.BindText(3, date.c_str(), (int)date.size());
	uint32_t db_reslut = stmt.NextRow();

	if (db_reslut == SQLITE_OK || db_reslut == SQLITE_ROW)
	{
		std::string count = stmt.GetTextField(0);
		return atoi(count.c_str());
	}
	return 0;
}

//void PublicDB::ReadForcePushData(const std::string userId, std::string date, std::map<std::string, std::string>& countMap)
//{
//
//}

void PublicDB::ReadCustomImgData(const std::string userId, std::list<CustomImgInfo>& imgInfoList)
{
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	//std::string strSql = "SELECT * FROM customImage where uid = " + userId;
	//db_.Query(stmt, strSql.data());
	db_.Query(stmt, "SELECT * FROM customImage WHERE uid=?");
	stmt.BindText(1, userId.c_str(), (int)userId.size());
	
	int32_t result = stmt.NextRow();
	while (result == SQLITE_ROW)
	{
		CustomImgInfo info;
		info.imgId = stmt.GetTextField(0);
		info.OriginalPath = stmt.GetTextField(2);
		info.thumbnailPath = stmt.GetTextField(3);
		imgInfoList.push_back(info);
		result = stmt.NextRow();
	}
	return;
}


bool PublicDB::WriteSessionItemTopData(SessionItemTopData &data)
{
	SessionItemTopData sessionitemdata;
	sessionitemdata = data;
	bool ret = this->GetSessionItemTopData(sessionitemdata);
	if (false == ret)
	{
		nbase::NAutoLock auto_lock(&lock_);
		ndb::SQLiteStatement stmt;
		db_.Query(stmt,
			"INSERT INTO item_top_info(uid, accid, value) VALUES (?, ?, ?);");

		stmt.BindText(1, data.uid.c_str(), (int)data.uid.size());
		stmt.BindText(2, data.accid.c_str(), (int)data.accid.size());
		stmt.BindInt(3, data.value);

		int32_t result = stmt.NextRow();
		bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
		if (false == no_error)
		{
			QLOG_APP(L"error: insert SessionItemTop data for uid : {0}, reason : {1}") << data.uid << result;
		}
		return no_error;
	}
	nbase::NAutoLock auto_lock(&lock_);
	UTF8String query_sql;
	nbase::StringPrintf(query_sql, "UPDATE OR ROLLBACK item_top_info SET value = '%d' \
								   								   	WHERE uid = '%s' AND accid = '%s'", data.value, shared::tools::FormatSQLText(data.uid).c_str(), shared::tools::FormatSQLText(data.accid).c_str());

	int32_t result = db_.Query(query_sql.c_str());
	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (!no_error)
	{
		QLOG_APP(L"Error: Set SessionItemTop user_header For uid: {0},  Reason : {1}") << data.uid << result;
	}

	return no_error;
}

bool PublicDB::GetSessionItemTopData(SessionItemTopData &data)
{
	nbase::NAutoLock auto_lock(&lock_);
	bool result = false;
	ndb::SQLiteStatement stmt;
	db_.Query(stmt, "SELECT * FROM item_top_info WHERE uid=? AND accid=?");
	stmt.BindText(1, data.uid.c_str(), (int)data.uid.size());
	stmt.BindText(2, data.accid.c_str(), (int)data.accid.size());
	uint32_t db_reslut = stmt.NextRow();

	if (db_reslut == SQLITE_OK || db_reslut == SQLITE_ROW)
	{
		GetSessionItemTopDataFromStatement(stmt, data);
		result = true;
	}
	return result;
}

void PublicDB::GetSessionItemTopDataFromStatement(ndb::SQLiteStatement &stmt, SessionItemTopData &data)
{
	data.uid = stmt.GetTextField(1);
	data.accid = stmt.GetTextField(2);
	data.value = stmt.GetIntField(3);
}

bool PublicDB::WriteLoginData(LoginData &data)
{
    nbase::NAutoLock auto_lock(&lock_);
    ndb::SQLiteStatement stmt;
	db_.Query(stmt,
		"INSERT INTO logindata(uid, name, password, type, status, remember_user, remember_psw, autologin, user_header) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);");

    stmt.BindText(1, data.user_id_.c_str(), (int)data.user_id_.size());
	stmt.BindText(2, data.user_name_.c_str(), (int)data.user_name_.size());
	UTF8String password_aes;
	GetAESPassword(data.user_password_, password_aes);
	stmt.BindText(3, password_aes.c_str(), (int)password_aes.size());
    stmt.BindInt(4, data.type_);
	stmt.BindInt(5, data.status_);
	stmt.BindInt(6, data.remember_user_);
	stmt.BindInt(7, data.remember_psw_);
	stmt.BindInt(8, data.auto_login_);
	stmt.BindText(9, data.user_header_.c_str(), (int)data.user_header_.size());

    int32_t result = stmt.NextRow();
    bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
    if (false == no_error)
    {
		QLOG_APP(L"error: insert login data for uid : {0}, reason : {1}") << data.user_id_ << result;
	}
    return no_error;
}

bool PublicDB::UpdateLoginData(std::string uid, std::string key, std::string value)
{
	/*nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	UTF8String query_sql;
	std::string sql = "UPDATE OR ROLLBACK logindata SET " + key + " = '%s' WHERE uid = '%s'";
	nbase::StringPrintf(query_sql, sql.c_str(),shared::tools::FormatSQLText(value).c_str(),shared::tools::FormatSQLText(uid).c_str());

	int32_t result = db_.Query(query_sql.c_str());
	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (!no_error)
	{
		QLOG_APP(L"Error: Set LoginData password For uid: {0}, Reason : {1}") << uid << result;
	}

	return no_error;*/
	LoginData login_data;
	bool ret = this->QueryLoginDataByUid(uid, login_data);
	if (false == ret)
	{
		return false;
	}
	nbase::NAutoLock auto_lock(&lock_);
	UTF8String query_sql;
	nbase::StringPrintf(query_sql, "UPDATE OR ROLLBACK logindata SET user_header = '%s' \
								   								   						 WHERE uid = '%s'", shared::tools::FormatSQLText(value).c_str(), shared::tools::FormatSQLText(uid).c_str());

	int32_t result = db_.Query(query_sql.c_str());
	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (!no_error)
	{
		QLOG_APP(L"Error: Set LoginData user_header For uid: {0},  Reason : {1}") << uid << result;
	}

	return no_error;
}

std::string PublicDB::GetUserHeaderPath(std::string uid)
{
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	db_.Query(stmt, "SELECT * FROM logindata WHERE uid=?");
	stmt.BindText(1, uid.c_str(), (int)uid.size());
	uint32_t db_reslut = stmt.NextRow();

	std::string path = "";
	if (db_reslut == SQLITE_OK || db_reslut == SQLITE_ROW)
	{
		//GetLoginDataFromStatement(stmt, data);
		path = stmt.GetIntField(8);
	}
	return path;
}

bool PublicDB::IsNeedUpdateData(const LoginData *orgi_login_data, 
	const LoginData *current_login_data, 
	bool &password_changed)
{
	if (0 != orgi_login_data->user_password_.compare(current_login_data->user_password_))
	{//密码已更改
		password_changed = true;
		return true;
	}
	else
	{
		password_changed = false;
	}

	if ((0 != orgi_login_data->user_name_.compare(current_login_data->user_name_)) ||
		(orgi_login_data->type_ != current_login_data->type_))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool PublicDB::UpdateLoginData(UTF8String &uid, 
	LoginData *current_login_data, 
	const uint8_t status, 
	bool password_changed)
{
    nbase::NAutoLock auto_lock(&lock_);
	UTF8String query_sql;
	if (true == password_changed)
	{
		UTF8String password_aes;
		GetAESPassword(current_login_data->user_password_, password_aes);
		nbase::StringPrintf(query_sql, "UPDATE OR ROLLBACK logindata SET name = '%s', password = '%s', type = %d, status = %d \
									   WHERE uid = '%s'", 
									   shared::tools::FormatSQLText(current_login_data->user_name_).c_str(),
									   password_aes.c_str(), 
									   current_login_data->type_, 
									   status, 
									   //shared::tools::FormatSQLText(current_login_data->user_header_).c_str(),
									   shared::tools::FormatSQLText(uid).c_str());
	}
	else
	{
		nbase::StringPrintf(query_sql, "UPDATE OR ROLLBACK logindata SET name = '%s', type = %d, status = %d \
									   WHERE uid = '%s'", 
									   shared::tools::FormatSQLText(current_login_data->user_name_).c_str(),
									   current_login_data->type_, 
									   status, 
									   shared::tools::FormatSQLText(uid).c_str());
	}
    
    int32_t result = db_.Query(query_sql.c_str());
    bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
    if (!no_error)
    {
		QLOG_APP(L"Error: Set LoginData password For uid: {0}, Reason : {1}") << uid << result;
	}
    
    return no_error;
}

bool PublicDB::SetStatus(UTF8String &uid, const uint8_t status)
{
	nbase::NAutoLock auto_lock(&lock_);
	UTF8String query_sql;
	nbase::StringPrintf(query_sql, "UPDATE OR ROLLBACK logindata SET status = %d \
								   WHERE uid = '%s'", status, shared::tools::FormatSQLText(uid).c_str());

	int32_t result = db_.Query(query_sql.c_str());
	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (!no_error)
	{
		QLOG_APP(L"Error: Set LoginData status For uid: {0},  Reason : {1}") << uid << result;
	}

	return no_error;
}

bool PublicDB::SetRemember(UTF8String &uid, const uint8_t remember_user, const uint8_t remember_psw)
{
	nbase::NAutoLock auto_lock(&lock_);
	UTF8String query_sql;
	nbase::StringPrintf(query_sql, "UPDATE OR ROLLBACK logindata SET remember_user = %d, remember_psw = %d \
								   WHERE uid = '%s'", remember_user, remember_psw, uid.c_str());

	int32_t result = db_.Query(query_sql.c_str());
	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (!no_error)
	{
		QLOG_APP(L"Error: Set LoginData remember For uid: {0}, Reason : {1}") << uid << result;
	}

	return no_error;
}

bool PublicDB::SetAutoLogin(UTF8String &uid, const uint8_t auto_login)
{
	nbase::NAutoLock auto_lock(&lock_);
	UTF8String query_sql;
	nbase::StringPrintf(query_sql, "UPDATE OR ROLLBACK logindata SET autologin = %d \
								   WHERE uid = '%s'", auto_login, shared::tools::FormatSQLText(uid).c_str());

	int32_t result = db_.Query(query_sql.c_str());
	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (!no_error)
	{
		QLOG_APP(L"Error: Set LoginData autologin For uid: {0}, Reason : {1}") << uid << result;
	}

	return no_error;
}

bool PublicDB::QueryLoginDataByUid(UTF8String &uid, LoginData &data)
{
    nbase::NAutoLock auto_lock(&lock_);
    bool result = false;
    ndb::SQLiteStatement stmt;
    db_.Query(stmt, "SELECT * FROM logindata WHERE uid=?");
	stmt.BindText(1, uid.c_str(), (int)uid.size());
    uint32_t db_reslut = stmt.NextRow();
    
    if (db_reslut == SQLITE_OK || db_reslut == SQLITE_ROW)
    {
        GetLoginDataFromStatement(stmt, data);
        result = true;
    }
    return result;
}


uint32_t PublicDB::QueryAllLoginData(std::vector<LoginData> &all_data)
{
    all_data.clear();
    nbase::NAutoLock auto_lock(&lock_);
    ndb::SQLiteStatement stmt;
    db_.Query(stmt, "SELECT * FROM logindata limit 10");
  
    int32_t result = stmt.NextRow();
    while (result == SQLITE_ROW)
    {
        LoginData login_data;
        GetLoginDataFromStatement(stmt, login_data);
        all_data.push_back(login_data);
        result = stmt.NextRow();
    }
    return (uint32_t)all_data.size();
}

bool PublicDB::CreateDBFile()
{
	if (db_.IsValid())
		return true;

    bool result = false;
	UTF8String dirctory = nbase::UTF16ToUTF8(QPath::GetNimAppDataDir(L""));
	
	UTF8String dbfile = dirctory + LOGIN_DATA_FILE;
	db_filepath_ = dbfile;
	result = db_.Open(dbfile.c_str(),
		db_encrypt_key_,
		ndb::SQLiteDB::modeReadWrite|ndb::SQLiteDB::modeCreate|ndb::SQLiteDB::modeSerialized
		); 
	if (result)
	{
		int dbresult = SQLITE_OK;
		for (size_t i = 0; i < kCreateDBSQLs.size(); i++)
		{
			dbresult |= db_.Query(kCreateDBSQLs[i].c_str());
		}
		result = dbresult == SQLITE_OK;
	}
    return result;
}

void PublicDB::GetLoginDataFromStatement(ndb::SQLiteStatement &stmt, LoginData &data)
{
	data.user_id_ = stmt.GetTextField(0);
	data.user_name_ = stmt.GetTextField(1);
	UTF8String password_org;
	UTF8String password_aes = stmt.GetTextField(2);
	GetOrgPassword(password_aes, password_org);
	data.user_password_ = password_org;
	data.type_ = stmt.GetIntField(3);
	data.status_ = stmt.GetIntField(4);
	data.remember_user_ = stmt.GetIntField(5);
	data.remember_psw_ = stmt.GetIntField(6);
	data.auto_login_ = stmt.GetIntField(7);
	data.user_header_ = stmt.GetTextField(8);
}

void PublicDB::GetAESPassword(const UTF8String &password_org, UTF8String &password_aes)
{
	nbase::EncryptInterface_var encrypt_enc(new nbase::Encrypt_Impl());
	encrypt_enc->SetMethod(nbase::ENC_AES128);
	encrypt_enc->SetEncryptKey(aes_key_);
	encrypt_enc->Encrypt(password_org, password_aes);
	password_aes = nbase::BinaryToHexString(password_aes);
}

void PublicDB::GetOrgPassword(const UTF8String &password_aes, UTF8String &password_org)
{
	nbase::EncryptInterface_var encrypt_dec(new nbase::Encrypt_Impl());
	encrypt_dec->SetMethod(nbase::ENC_AES128);
	encrypt_dec->SetDecryptKey(aes_key_);
	std::string password_enc = nbase::HexStringToBinary(password_aes);
	encrypt_dec->Decrypt(password_enc, password_org);
}

bool PublicDB::SetAllLoginDataDeleted()
{
	nbase::NAutoLock auto_lock(&lock_);
	UTF8String query_sql;
	nbase::StringPrintf(query_sql, "UPDATE OR ROLLBACK logindata SET status = %d",
		kLoginDataStatusDeleted);
	int32_t result = db_.Query(query_sql.c_str());
	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (!no_error)
	{
		QLOG_APP(L"Error: Set All Login Data Deleted");
	}
	return no_error;
}

void PublicDB::ReadLoginData()
{
	bool bfind = true;
	try
	{
		std::vector<LoginData> all_data;
		this->QueryAllLoginData(all_data);
		if (all_data.size() > 0)
		{
			std::vector<LoginData>::iterator it = all_data.begin();
			for (; it != all_data.end(); it++)
			{
				if (kLoginDataStatusValid == it->status_)
				{
					if (bfind)
					{
						bfind = false;
						current_login_data_ = *it;
					}
					rember_login_data_.push_back(*it);
				}
			}
		}
	}
	catch (...)
	{

	}
}
void PublicDB::DelLoginData(std::string uid){
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	std::string sql = "delete from logindata where name='" + uid + "'";
	db_.Query(stmt, sql.c_str());
	stmt.NextRow();
}
void PublicDB::SaveLoginData()
{
	LoginData login_data;
	bool ret = this->QueryLoginDataByUid(current_login_data_.user_id_, login_data);
	if (false == ret)
	{
		//不存在，则直接写入，并将原来的登录帐号全部设为已删除
		//this->SetAllLoginDataDeleted();
		this->WriteLoginData(current_login_data_);
	}
	else
	{
		//this->SetAllLoginDataDeleted();
		bool password_changed = false;
		if (true == this->IsNeedUpdateData(&login_data, &current_login_data_, password_changed))
		{
			this->UpdateLoginData(current_login_data_.user_id_,
				&current_login_data_,
				kLoginDataStatusValid,
				password_changed);
		}
		else
		{
			//只更改状态
			this->SetStatus(current_login_data_.user_id_, kLoginDataStatusValid);
		}
		//是否记住帐号密码
		this->SetRemember(current_login_data_.user_id_, current_login_data_.remember_user_, current_login_data_.remember_psw_);
	}
}

bool PublicDB::InsertConfigData(const std::string& key, const std::string& value)
{
	nbase::NAutoLock auto_lock(&lock_);

	ndb::SQLiteStatement stmt;
	db_.Query(stmt, "INSERT OR REPLACE into config_info (key, value) values (?, ?);");
	stmt.BindText(1, key.c_str(), (int)key.size());
	stmt.BindText(2, value.c_str(), (int)value.size());
	int32_t result = stmt.NextRow();
	stmt.Finalize();

	bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
	if (!no_error)
	{
		QLOG_ERR(L"error: InsertConfigData for key: {0}, reason: {1}") << key << result;
	}

	return no_error;
}

void PublicDB::QueryConfigData(const std::string& key, std::string& value)
{
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;

	db_.Query(stmt, "SELECT value FROM config_info WHERE key=?");
	stmt.BindText(1, key.c_str(), (int)key.size());
	int32_t result = stmt.NextRow();

	if (result == SQLITE_ROW)
		value = stmt.GetTextField(0);
}

void PublicDB::ClearConfigData()
{
	nbase::NAutoLock auto_lock(&lock_);

	ndb::SQLiteStatement stmt;
	db_.Query(stmt, "delete from config_info;");
	stmt.NextRow();
}
//操作接口
void PublicDB::UrlGroup(int ntype, std::string sname, std::string accid)
{
	//新增
	if (ntype == 0)
	{
		std::string body;
		body += "action=add&accid="+accid;
		body += "&uid=" + nim_ui::LoginManager::GetInstance()->GetAccount() + "&token=" + nim_ui::LoginManager::GetInstance()->GetAccountToken();
		body += "&name="+sname;
		body += "&content=[]";
		std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
		nim_http::HttpRequest request(app_sdk::AppSDKInterface::GetInstance()->GetAppHost() + "/api/group/setgroup", body.c_str(), body.size(), function<void(bool, int, const std::string&)>([this](bool ret, int response_code, const std::string& reply) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (res)
			{
				std::string id = json["data"]["id"].asString();
				std::string name = json["data"]["name"].asString();
				std::string accid = json["data"]["accid"].asString();
				std::string sql = "update friend_group set id='" + id + "' where groupname='" + name + "' and macct='" + accid + "'";
				db_.Query(sql.c_str());
			}
			 
			
		}));
		request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
		request.AddHeader("charset", "utf-8");
		request.AddHeader("appkey", app_key);
		request.AddHeader("User-Agent", "nim_demo_pc");
		request.SetMethodAsPost();
		nim_http::PostRequest(request);
	}
	else if (ntype == 1)//更新
	{
		std::string id = "";
		std::string fid = "";
		std::string json_msg = "[";
		int i = 0;
		nbase::NAutoLock auto_lock(&lock_);
		ndb::SQLiteStatement stmt;
		std::string sql = "SELECT id,friendacct FROM friend_group WHERE groupname='" + sname + "' and macct='" + accid + "'";
		db_.Query(stmt, sql.c_str());
		int32_t result = stmt.NextRow();
		while (result == SQLITE_ROW)
		{
			fid = stmt.GetTextField(1);
			if (fid == ""){
				id = stmt.GetTextField(0);
			}
			else{
				if (i == 0){
					json_msg = json_msg + "'" + fid + "'";
				}
				else
				{
					json_msg = json_msg + ",'" + fid + "'";
				}
				i = 1;
			}
			result = stmt.NextRow();
		}
		if (!id.empty()){
			json_msg = json_msg + "]";
			std::string body;
			body += "action=edit&accid=" + accid + "&id=" + id;
			body += "&uid=" + nim_ui::LoginManager::GetInstance()->GetAccount() + "&token=" + nim_ui::LoginManager::GetInstance()->GetAccountToken();
			body += "&name=" + sname;
			body += "&content=" + json_msg;
			std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
			nim_http::HttpRequest request(app_sdk::AppSDKInterface::GetInstance()->GetAppHost() + "/api/group/setgroup", body.c_str(), body.size(),nullptr);
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.AddHeader("charset", "utf-8");
			request.AddHeader("appkey", app_key);
			request.AddHeader("User-Agent", "nim_demo_pc");
			request.SetMethodAsPost();
			nim_http::PostRequest(request);
		}
	}
	else if (ntype == -1)//删除
	{
		std::string id = "";
		nbase::NAutoLock auto_lock(&lock_);
		ndb::SQLiteStatement stmt;
		std::string sql = "SELECT id FROM friend_group WHERE groupname=? and macct=?";
		stmt.BindText(1, sname.c_str(), (int)sname.size());
		stmt.BindText(2, accid.c_str(), (int)accid.size());
		db_.Query(stmt, sql.c_str());
		int32_t result = stmt.NextRow();
		while (result == SQLITE_ROW)
		{
			id = stmt.GetTextField(0);
			if (!id.empty()){
				std::string body;
				body += "action=del&id=" + id;
				body += "&uid=" + nim_ui::LoginManager::GetInstance()->GetAccount() + "&token=" + nim_ui::LoginManager::GetInstance()->GetAccountToken();
				std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
				nim_http::HttpRequest request(app_sdk::AppSDKInterface::GetInstance()->GetAppHost() + "/api/group/setgroup", body.c_str(), body.size(), nullptr);
				request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
				request.AddHeader("charset", "utf-8");
				request.AddHeader("appkey", app_key);
				request.AddHeader("User-Agent", "nim_demo_pc");
				request.SetMethodAsPost();
				nim_http::PostRequest(request);
			}
			result = stmt.NextRow();
		} 
		sql = "delete from friend_group  where groupname='" + sname + "' and macct='" + accid + "'";
		db_.Query(sql.c_str());
		
	}
}

void PublicDB::AddFriendGroup(std::string myaccid, std::string oldname, std::string sname, std::string accid)
{
	if (oldname != ""){//重命名
		std::string sql = "update friend_group set groupname='"+sname+"' where groupname='"+oldname+"' and macct='"+myaccid+"'";
		db_.Query(sql.c_str()); 
		UrlGroup(1, sname, myaccid);
	}
	else if (accid == "")//创建组
	{
		std::string sql = "select groupname from friend_group  where groupname='" + sname + "' and macct='" + myaccid + "' limit 1";
		nbase::NAutoLock auto_lock(&lock_);
		ndb::SQLiteStatement stmt;
		db_.Query(stmt, sql.c_str());
		int32_t result = stmt.NextRow();
		if (result == SQLITE_ROW){
			accid = stmt.GetTextField(0);
		}
		else
		{
			nbase::NAutoLock auto_lock(&lock_);
			ndb::SQLiteStatement stmt2;
			db_.Query(stmt2, "INSERT into friend_group (macct,groupname,friendacct) values (?, ?, ?);");
			stmt2.BindText(1, myaccid.c_str(), (int)myaccid.size());
			stmt2.BindText(2, sname.c_str(), (int)sname.size());
			stmt2.BindText(3, accid.c_str(), (int)accid.size());
			int32_t result = stmt2.NextRow();
			bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
			if (false == no_error)
			{
				
			}
			UrlGroup(0, sname, myaccid);
		}
	}
	else
	{
		nbase::NAutoLock auto_lock(&lock_);
		ndb::SQLiteStatement stmt3;
		std::string sql = "delete from friend_group where macct='" + myaccid + "' and friendacct='" + accid + "'";
		db_.Query(stmt3,sql.c_str());
		ndb::SQLiteStatement stmt2;
		db_.Query(stmt2, "INSERT into friend_group (macct,groupname,friendacct) values (?, ?, ?);");
		stmt2.BindText(1, myaccid.c_str(), (int)myaccid.size());
		stmt2.BindText(2, sname.c_str(), (int)sname.size());
		stmt2.BindText(3, accid.c_str(), (int)accid.size());
		int32_t result = stmt2.NextRow();
		bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
		if (false == no_error)
		{
			
		} 
	}
}

void PublicDB::DelGroup(std::string myaccid, std::string sname)
{
	/*std::string sql = "delete from friend_group  where groupname='" + sname + "' and macct='" + myaccid + "'";
	db_.Query(sql.c_str());*/
	UrlGroup(-1, sname, myaccid);
}


void PublicDB::CheckGroup(){
	//单独判读分组的 kCreateDBSQLs.push_back();
	std::string sql = "SELECT count(*) from sqlite_master where type='table' and name='friend_group'";
	ndb::SQLiteStatement stmt;
	db_.Query(stmt, sql.c_str());
	int32_t resultx = stmt.NextRow();
	if (resultx == SQLITE_ROW){
		UTF8String v = stmt.GetTextField(0);
		if (v == "0"){
			std::string addsql = "CREATE TABLE IF NOT EXISTS friend_group(id TEXT,macct TEXT,groupname TEXT,friendacct TEXT)";
			db_.Query(addsql.c_str());
			//从接口获取历史数据 并更新
			std::string body = "uid=" + nim_ui::LoginManager::GetInstance()->GetAccount() + "&token=" + nim_ui::LoginManager::GetInstance()->GetAccountToken();
			std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
			nim_http::HttpRequest request(app_sdk::AppSDKInterface::GetInstance()->GetAppHost() + "/api/group/getgroups", body.c_str(), body.size(), function<void(bool, int, const std::string&)>([this](bool ret, int response_code, const std::string& reply) {
				Json::Value json;
				Json::Reader reader;
				bool res = reader.parse(reply, json);
				if (res)
				{
					nbase::NAutoLock auto_lock(&lock_);
					ndb::SQLiteStatement stmt2;
					Json::Value qs = json["data"];
					int sz = (int)qs.size();
					for (auto i = 0; i < sz; i++)
					{
						std::string id = nbase::IntToString(qs[i]["id"].asInt());
						std::string name = qs[i]["name"].asString();
						std::string us = qs[i]["content"].asString();
						std::string accid = qs[i]["user_id"].asString();
						
						std::string accid2 = "";
						db_.Query(stmt2, "INSERT into friend_group (id,macct,groupname,friendacct) values (?, ?, ?, ?);");
						stmt2.BindText(1, id.c_str(), (int)id.size());
						stmt2.BindText(2, accid.c_str(), (int)accid.size());
						stmt2.BindText(3, name.c_str(), (int)name.size());
						stmt2.BindText(4, accid2.c_str(), (int)accid2.size());
						int32_t result = stmt2.NextRow();
						if (us != "[]"){
							nbase::StringReplaceAll("['", "", us);
							nbase::StringReplaceAll("']", "", us);
							vector<string> rs;
							int cutAt;
							std::string separator = "','";
							while ((cutAt = us.find_first_of(separator)) != us.npos){
								if (cutAt>0){
									rs.push_back(us.substr(0, cutAt));
								}
								us = us.substr(cutAt + 1);
							}
							if (us.length()>0){
								rs.push_back(us);
							}

							int rz = (int)rs.size();
							for (auto j = 0; j < rz; j++)
							{
								std::string fid = rs[j];
								db_.Query(stmt2, "INSERT into friend_group (id,macct,groupname,friendacct) values (?, ?, ?, ?);");
								stmt2.BindText(1, id.c_str(), (int)id.size());
								stmt2.BindText(2, accid.c_str(), (int)accid.size());
								stmt2.BindText(3, name.c_str(), (int)name.size());
								stmt2.BindText(4, fid.c_str(), (int)fid.size());
								int32_t result = stmt2.NextRow();

							}
						}
					}

				}


			}));
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.AddHeader("charset", "utf-8");
			request.AddHeader("appkey", app_key);
			request.AddHeader("User-Agent", "nim_demo_pc");
			request.SetMethodAsPost();
			nim_http::PostRequest(request);
		}
	}
}

std::list<pair<std::string, std::string>> PublicDB::GetGroupFriend(std::string myaccid)
{
	std::list<pair<std::string, std::string>> da;
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	//std::string strSql = "SELECT * FROM customImage where uid = " + userId;
	//db_.Query(stmt, strSql.data());
	db_.Query(stmt, "SELECT groupname,friendacct FROM friend_group WHERE macct=?");
	stmt.BindText(1, myaccid.c_str(), (int)myaccid.size());

	int32_t result = stmt.NextRow();
	while (result == SQLITE_ROW)
	{
		da.push_back(make_pair(stmt.GetTextField(0), stmt.GetTextField(1)));
		result = stmt.NextRow();
	} 
	return da;
}
std::list<pair<std::string, std::string>> PublicDB::GetGroupFriendByName(std::string myaccid,std::string sname)
{
	std::list<pair<std::string, std::string>> da;
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	//std::string strSql = "SELECT * FROM customImage where uid = " + userId;
	//db_.Query(stmt, strSql.data());
	db_.Query(stmt, "SELECT groupname,friendacct FROM friend_group WHERE macct=? and groupname=?");
	stmt.BindText(1, myaccid.c_str(), (int)myaccid.size());
	stmt.BindText(1, sname.c_str(), (int)sname.size());
	int32_t result = stmt.NextRow();
	while (result == SQLITE_ROW)
	{
		da.push_back(make_pair(stmt.GetTextField(0), stmt.GetTextField(1)));
		result = stmt.NextRow();
	}
	return da;
}

bool PublicDB::AddRecord(std::string stype, std::string svalue){
	//item_value
	std::string sql = "select svalue from item_value  where skey='" + stype + "' and svalue='"+svalue+"' limit 1";
	nbase::NAutoLock auto_lock(&lock_);
	ndb::SQLiteStatement stmt;
	db_.Query(stmt, sql.c_str());
	int32_t result = stmt.NextRow();
	if (result == SQLITE_ROW){
		return false;
	}
	else{
		ndb::SQLiteStatement stmt2;
		db_.Query(stmt2, "INSERT into item_value (skey,svalue) values (?, ?);");
		stmt2.BindText(1, stype.c_str(), (int)stype.size());
		stmt2.BindText(2, svalue.c_str(), (int)svalue.size());
		int32_t result = stmt2.NextRow();
		bool no_error = result == SQLITE_OK || result == SQLITE_ROW || result == SQLITE_DONE;
		if (false == no_error)
		{

		}
	}
	return true;
}