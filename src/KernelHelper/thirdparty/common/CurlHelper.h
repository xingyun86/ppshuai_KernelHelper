
#define CURL_STATICLIB
#include <curl/curl.h>

#include <string>
using namespace std;

namespace CURLTOOL {

	typedef struct _tagCallBackData {
#define call_back_data_size 0x10000
		char * p;
		unsigned int s;
		unsigned int v;
		static void * startup()
		{
			void * thiz = malloc(sizeof(struct _tagCallBackData));
			if (thiz)
			{
				((struct _tagCallBackData *)thiz)->init();
			}
			return thiz;
		}
		_tagCallBackData()
		{
		}
		_tagCallBackData(char ** _p = 0, unsigned int _s = 0, unsigned int _v = 0)
		{
			init(_p, _s, _v);
		}
		void init(char ** _p = 0, unsigned int _s = 0, unsigned int _v = 0)
		{
			p = _p ? (*_p) : 0; s = _s; v = (!p || !_v) ? call_back_data_size : _v;
			if (!p)
			{
				p = (char *)malloc(v * sizeof(char));
			}
			if (p && v > 0)
			{
				memset(p, 0, v * sizeof(char));
			}
		}
		void copy(const char * _p, unsigned int _s)
		{
			if (_s > 0)
			{
				if (s + _s > v)
				{
					v += _s + 1;
					p = (char *)realloc(p, v * sizeof(char));
					memset(p + s, 0, _s + 1);
				}
				if (p)
				{
					memcpy(p, _p, _s);
					s = _s;
				}
			}
		}
		char * append(char * _p, unsigned int _s)
		{
			if (_s > 0)
			{
				if (s + _s > v)
				{
					v += _s + 1;
					p = (char *)realloc(p, v * sizeof(char));
					memset(p + s, 0, _s + 1);
				}
				if (p)
				{
					memcpy(p + s, _p, _s);
					s += _s;
				}
			}
			return p;
		}
		void exit(char ** _p)
		{
			if (_p && (*_p))
			{
				free((*_p));
				(*_p) = 0;
			}
			s = v = 0;
		}
		void cleanup()
		{
			exit(&p);
			free(this);
		}
	}CALLBACKDATA, *PCALLBACKDATA;
	typedef struct tagDebugBlock {
		char trace_ascii; /* 1 or 0 */
		tagDebugBlock(char _trace_ascii)
		{
			this->trace_ascii = _trace_ascii;
		}
	}DEBUGBLOCK, *PDEBUGBLOCK;

	static __inline size_t read_callback(void *p_data, size_t n_size, size_t n_memb, void *p_argv)
	{
		CALLBACKDATA *l = (CALLBACKDATA *)p_argv;
		/* take care of the packet in 'ptr', then return... */
		return n_size * n_memb;
	}
	static __inline size_t write_callback(void *p_data, size_t n_size, size_t n_memb, void *p_argv)
	{
		CALLBACKDATA *l = (CALLBACKDATA *)p_argv;
		/* take care of the packet in 'ptr', then return... */
		return n_size * n_memb;
	}
	static __inline void dump_data(const char * p_text, FILE * p_stream, char *p_data, size_t n_size, char nohex)
	{
		size_t i = 0;
		size_t c = 0;

		unsigned int width = 0x10;

		if (nohex)
		{
			/* without the hex output, we can fit more on screen */
			width = 0x40;
		}

		fprintf(p_stream, "%s, %10.10ld bytes (0x%8.8lx)\n", p_text, (long)n_size, (long)n_size);

		for (i = 0; i < n_size; i += width) {

			fprintf(p_stream, "%4.4lx: ", (long)i);

			if (!nohex) {
				/* hex not disabled, show it */
				for (c = 0; c < width; c++)
				{
					if (i + c < n_size)
					{
						fprintf(p_stream, "%02X ", p_data[i + c]);
					}
					else
					{
						fputs("   ", p_stream);
					}
				}
			}

			for (c = 0; (c < width) && (i + c < n_size); c++) {
				/* check for 0D0A; if found, skip past and start a new line of output */
				if (nohex && (i + c + 1 < n_size) && p_data[i + c] == 0x0D &&
					p_data[i + c + 1] == 0x0A) {
					i += (c + 2 - width);
					break;
				}
				fprintf(p_stream, "%c",
					(p_data[i + c] >= 0x20) && (p_data[i + c]<0x80) ? p_data[i + c] : '.');
				/* check again for 0D0A, to avoid an extra \n if it's at width */
				if (nohex && (i + c + 2 < n_size) && p_data[i + c + 1] == 0x0D &&
					p_data[i + c + 2] == 0x0A) {
					i += (c + 3 - width);
					break;
				}
			}
			fputc('\n', p_stream); /* newline */
		}
		fflush(p_stream);
	}
	static __inline int debug_callback(CURL * p_curl, curl_infotype info_type, char *p_data, size_t n_size, void *p_argv)
	{
		DEBUGBLOCK *p_db = (DEBUGBLOCK *)p_argv;
		const char *p_text = 0;
		(void)p_curl; /* prevent compiler warning */

		switch (info_type) {
		case CURLINFO_TEXT:
			fprintf(stderr, "== Info: %s", p_data);
			/* FALLTHROUGH */
		default: /* in case a new one is introduced to shock us */
			return 0;

		case CURLINFO_HEADER_OUT:
			p_text = "=> Send header";
			break;
		case CURLINFO_DATA_OUT:
			p_text = "=> Send data";
			break;
		case CURLINFO_SSL_DATA_OUT:
			p_text = "=> Send SSL data";
			break;
		case CURLINFO_HEADER_IN:
			p_text = "<= Recv header";
			break;
		case CURLINFO_DATA_IN:
			p_text = "<= Recv data";
			break;
		case CURLINFO_SSL_DATA_IN:
			p_text = "<= Recv SSL data";
			break;
		}

		dump_data(p_text, stderr, p_data, n_size, p_db->trace_ascii);

		return 0;
	}
	static __inline size_t header_callback(void * p_data, size_t n_size, size_t n_menb, void * p_argv)
	{
		int result = 0;

		return n_size * n_menb;
	}
	static __inline CURLcode sslctx_callback(CURL *p_curl, void *sslctx, void *p_argv)
	{
		//X509_STORE *store;
		//X509 *cert = NULL;
		//BIO *bio;
		//char *mypem = /* example CA cert PEM - shortened */
		//	"-----BEGIN CERTIFICATE-----\n"
		//	"MIIHPTCCBSWgAwIBAgIBADANBgkqhkiG9w0BAQQFADB5MRAwDgYDVQQKEwdSb290\n"
		//	"IENBMR4wHAYDVQQLExVodHRwOi8vd3d3LmNhY2VydC5vcmcxIjAgBgNVBAMTGUNB\n"
		//	"IENlcnQgU2lnbmluZyBBdXRob3JpdHkxITAfBgkqhkiG9w0BCQEWEnN1cHBvcnRA\n"
		//	"Y2FjZXJ0Lm9yZzAeFw0wMzAzMzAxMjI5NDlaFw0zMzAzMjkxMjI5NDlaMHkxEDAO\n"
		//	"GCSNe9FINSkYQKyTYOGWhlC0elnYjyELn8+CkcY7v2vcB5G5l1YjqrZslMZIBjzk\n"
		//	"zk6q5PYvCdxTby78dOs6Y5nCpqyJvKeyRKANihDjbPIky/qbn3BHLt4Ui9SyIAmW\n"
		//	"omTxJBzcoTWcFbLUvFUufQb1nA5V9FrWk9p2rSVzTMVD\n"
		//	"-----END CERTIFICATE-----\n";
		/* get a BIO */
		//bio = BIO_new_mem_buf(mypem, -1);
		/* use it to read the PEM formatted certificate from memory into an
		* X509 structure that SSL can use
		*/
		//PEM_read_bio_X509(bio, &cert, 0, NULL);
		//if (cert == NULL)
		//	printf("PEM_read_bio_X509 failed...\n");

		/* get a pointer to the X509 certificate store (which may be empty) */
		//store = SSL_CTX_get_cert_store((SSL_CTX *)sslctx);

		/* add our certificate to this store */
		//if (X509_STORE_add_cert(store, cert) == 0)
		//	printf("error adding certificate\n");

		/* decrease reference counts */
		//X509_free(cert);
		//BIO_free(bio);

		/* all set to go */
		return CURLE_OK;
	}
	static __inline curlioerr ioctl_callback(CURL *p_curl, int n_cmd, void *p_argv)
	{
		struct CALLBACKDATA *io = (struct CALLBACKDATA *)p_argv;
		if (n_cmd == CURLIOCMD_RESTARTREAD)
		{
			//lseek(fd, 0, SEEK_SET);
			//current_offset = 0;
			return CURLIOE_OK;
		}
		return CURLIOE_UNKNOWNCMD;
	}
	static __inline CURLcode conv_from_network_callback(char *p_data, size_t n_size)
	{
		char *tempptrin = 0, *tempptrout = 0;
		size_t bytes = n_size;
		int rc = 0;
		tempptrin = tempptrout = p_data;
		//rc = platform_e2a(&tempptrin, &bytes, &tempptrout, &bytes);
		//if (rc == PLATFORM_CONV_OK) 
		{
			return CURLE_OK;
		}
		//else 
		{
			return CURLE_CONV_FAILED;
		}
	}
	static __inline CURLcode conv_to_network_callback(char *p_data, size_t n_size)
	{
		char *tempptrin = 0, *tempptrout = 0;
		size_t bytes = n_size;
		int rc = 0;
		tempptrin = tempptrout = p_data;
		//rc = platform_a2e(&tempptrin, &bytes, &tempptrout, &bytes);
		//if (rc == PLATFORM_CONV_OK) 
		{
			return CURLE_OK;
		}
		//else 
		{
			return CURLE_CONV_FAILED;
		}
	}
	/* make libcurl use the already established socket 'sockfd' */
	static __inline curl_socket_t opensocket_callback(void *p_argv, curlsocktype purpose, struct curl_sockaddr *address)
	{
		curl_socket_t sockfd;
		sockfd = *(curl_socket_t *)p_argv;
		/* the actual externally set socket is passed in via the OPENSOCKETDATA
		option */
		return sockfd;
	}
	static __inline int sockopt_callback(void *p_argv, curl_socket_t curlfd, curlsocktype purpose)
	{
		int val = *(int *)p_argv;
		setsockopt(curlfd, SOL_SOCKET, SO_RCVBUF, (const char *)&val, sizeof(val));
		return CURL_SOCKOPT_OK;
	}
	static __inline CURLcode conv_from_utf8_callback(char *p_data, size_t n_size)
	{
		char *tempptrin = 0, *tempptrout = 0;
		size_t bytes = n_size;
		int rc = 0;
		tempptrin = tempptrout = p_data;
		//rc = platform_a2e(&tempptrin, &bytes, &tempptrout, &bytes);
		//if (rc == PLATFORM_CONV_OK) 
		{
			return CURLE_OK;
		}
		//else 
		{
			return CURLE_CONV_FAILED;
		}
	}
	static __inline int seek_callback(void *p_argv, curl_off_t offset, int origin)
	{
		CALLBACKDATA * p_cbd = (CALLBACKDATA *)p_argv;
		//lseek(our_fd, offset, origin);
		return CURL_SEEKFUNC_OK;
	}
	static __inline int ssh_key_callback(CURL *easy, const struct curl_khkey *knownkey, const struct curl_khkey *foundkey, enum curl_khmatch, void *p_argv)
	{
		/* 'clientp' points to the callback_data struct */
		/* investigate the situation and return the correct value */
		return CURLKHSTAT_FINE_ADD_TO_FILE;
	}
	static __inline size_t interleavedata_callback(void *p_data, size_t n_size, size_t n_memb, void *p_argv)
	{
		CALLBACKDATA *l = (CALLBACKDATA *)p_argv;
		/* take care of the packet in 'ptr', then return... */
		return n_size * n_memb;
	}
	static __inline long chunk_bgn_callback(struct curl_fileinfo *finfo, void *p_argv, int remains)
	{
		printf("%3d %40s %10luB ", remains, finfo->filename,
			(unsigned long)finfo->size);

		switch (finfo->filetype) {
		case CURLFILETYPE_DIRECTORY:
			printf(" DIRn");
			break;
		case CURLFILETYPE_FILE:
			printf("FILE ");
			break;
		default:
			printf("OTHERn");
			break;
		}

		if (finfo->filetype == CURLFILETYPE_FILE) {
			/* do not transfer files >= 50B */
			if (finfo->size > 50) {
				printf("SKIPPEDn");
				return CURL_CHUNK_BGN_FUNC_SKIP;
			}

			//data->output = fopen(finfo->filename, "wb");
			//if (!data->output) 
			//{
			//	return CURL_CHUNK_BGN_FUNC_FAIL;
			//}
		}

		return CURL_CHUNK_BGN_FUNC_OK;
	}
	static __inline long chunk_end_callback(void * p_argv)
	{
		//if (data->output)
		//{
		//	fclose(data->output);
		//	data->output = 0x0;
		//}
		return CURL_CHUNK_END_FUNC_OK;
	}
	static __inline int fnmatch_callback(void *p_argv, const char *pattern, const char *string)
	{
		CALLBACKDATA *pCBD = (CALLBACKDATA *)p_argv;
		//if (string_match(pattern, string))
		{
			//	return CURL_FNMATCHFUNC_MATCH;
		}
		//else
		{
			return CURL_FNMATCHFUNC_NOMATCH;
		}
	}
	static __inline int closesocket_callback(void *p_argv, curl_socket_t sfd)
	{
		printf("libcurl wants to close %d nown", (int)sfd);
		return 0;
	}
	static __inline void curl_setopt(CURL *p_curl)
	{
		/* This is the FILE * or void * the regular output should be written to. */
		curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, (CALLBACKDATA*)0L);

		/* The full URL to get/put */
		curl_easy_setopt(p_curl, CURLOPT_URL, (""));

		/* Port number to connect to, if other than default. */
		curl_easy_setopt(p_curl, CURLOPT_PORT, (-1L));

		/* Name of proxy to use. */
		curl_easy_setopt(p_curl, CURLOPT_PROXY, (""));

		/* "user:password;options" to use when fetching. */
		curl_easy_setopt(p_curl, CURLOPT_USERPWD, (""));

		/* "user:password" to use with proxy. */
		curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, (""));

		/* Range to get, specified as an ASCII string. */
		curl_easy_setopt(p_curl, CURLOPT_RANGE, (""));

		/* not used */

		/* Specified file stream to upload from (use as input): */
		curl_easy_setopt(p_curl, CURLOPT_READDATA, (CALLBACKDATA *)0L);

		/* Buffer to receive error messages in, must be at least CURL_ERROR_SIZE
		* bytes big. If this is not used, error messages go to stderr instead: */
		curl_easy_setopt(p_curl, CURLOPT_ERRORBUFFER, (CALLBACKDATA*)0L);

		/* Function that will be called to store the output (instead of fwrite). The
		* parameters will use fwrite() syntax, make sure to follow them. */
		//curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, [](void * p_data, size_t n_size, size_t n_menb, void * p_argv){});
		curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, write_callback);

		/* Function that will be called to read the input (instead of fread). The
		* parameters will use fread() syntax, make sure to follow them. */
		//curl_easy_setopt(p_curl, CURLOPT_READFUNCTION, [](void * p_data, size_t n_size, size_t n_menb, void * p_argv){});
		curl_easy_setopt(p_curl, CURLOPT_READFUNCTION, read_callback);

		/* Time-out the read operation after this amount of seconds */
		curl_easy_setopt(p_curl, CURLOPT_TIMEOUT, (0L));

		/* Time-out the read operation after this amount of milliseconds */
		curl_easy_setopt(p_curl, CURLOPT_TIMEOUT_MS, (0L));

		/* If the CURLOPT_INFILE is used, this can be used to inform libcurl about
		* how large the file being sent really is. That allows better error
		* checking and better verifies that the upload was successful. -1 means
		* unknown size.
		*
		* For large file support, there is also a _LARGE version of the key
		* which takes an off_t type, allowing platforms with larger off_t
		* sizes to handle larger files.  See below for INFILESIZE_LARGE.
		*/
		curl_easy_setopt(p_curl, CURLOPT_INFILESIZE, (-1L));

		/* POST static input fields. */
		curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, (CALLBACKDATA*)0L);

		/* Set the referrer page (needed by some CGIs) */
		curl_easy_setopt(p_curl, CURLOPT_REFERER, (""));

		/* Set the FTP PORT string (interface name, named or numerical IP address)
		Use i.e '-' to use default address. */
		curl_easy_setopt(p_curl, CURLOPT_FTPPORT, ("-"));

		/* Set the User-Agent string (examined by some CGIs) */
		curl_easy_setopt(p_curl, CURLOPT_USERAGENT, (""));

		/* If the download receives less than "low speed limit" bytes/second
		* during "low speed time" seconds, the operations is aborted.
		* You could i.e if you have a pretty high speed connection, abort if
		* it is less than 2000 bytes/sec during 20 seconds.
		*/

		/* Set the "low speed limit" */
		curl_easy_setopt(p_curl, CURLOPT_LOW_SPEED_LIMIT, (-1L));

		/* Set the "low speed time" */
		curl_easy_setopt(p_curl, CURLOPT_LOW_SPEED_TIME, (-1L));

		/* Set the continuation offset.
		*
		* Note there is also a _LARGE version of this key which uses
		* off_t types, allowing for large file offsets on platforms which
		* use larger-than-32-bit off_t's.  Look below for RESUME_FROM_LARGE.
		*/
		curl_easy_setopt(p_curl, CURLOPT_RESUME_FROM, (-1L));

		/* Set cookie in request: */
		curl_easy_setopt(p_curl, CURLOPT_COOKIE, (""));

		/* This points to a linked list of headers, struct curl_slist kind. This
		list is also used for RTSP (in spite of its name) */
		curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, (struct curl_slist *)0L);

		/* This points to a linked list of post entries, struct curl_httppost */
		curl_easy_setopt(p_curl, CURLOPT_HTTPPOST, (struct curl_httppost *)0L);

		/* name of the file keeping your private SSL-certificate */
		curl_easy_setopt(p_curl, CURLOPT_SSLCERT, (""));

		/* password for the SSL or SSH private key */
		curl_easy_setopt(p_curl, CURLOPT_KEYPASSWD, (""));

		/* send TYPE parameter? */
		curl_easy_setopt(p_curl, CURLOPT_CRLF, (-1L));

		/* send linked-list of QUOTE commands */
		curl_easy_setopt(p_curl, CURLOPT_QUOTE, (struct curl_slist *)0L);

		/* send FILE * or void * to store headers to, if you use a callback it
		is simply passed to the callback unmodified */
		curl_easy_setopt(p_curl, CURLOPT_HEADERDATA, (CALLBACKDATA*)0L);

		/* point to a file to read the initial cookies from, also enables
		"cookie awareness" */
		curl_easy_setopt(p_curl, CURLOPT_COOKIEFILE, (""));

		/* What version to specifically try to use.
		See CURL_SSLVERSION defines below. */
		curl_easy_setopt(p_curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);

		/* What kind of HTTP time condition to use, see defines */
		curl_easy_setopt(p_curl, CURLOPT_TIMECONDITION, CURL_TIMECOND_NONE);

		/* Time to use with the above condition. Specified in number of seconds
		since 1 Jan 1970 */
		curl_easy_setopt(p_curl, CURLOPT_TIMEVALUE, (-1L));

		/* 35 = OBSOLETE */

		/* Custom request, for customizing the get command like
		HTTP: DELETE, TRACE and others
		FTP: to use a different list command
		*/
		curl_easy_setopt(p_curl, CURLOPT_CUSTOMREQUEST, (""));

		/* FILE handle to use instead of stderr */
		curl_easy_setopt(p_curl, CURLOPT_STDERR, (struct _iobuf*)stderr);

		/* 38 is not used */

		/* send linked-list of post-transfer QUOTE commands */
		curl_easy_setopt(p_curl, CURLOPT_POSTQUOTE, (struct curl_slist*)0L);

		//curl_easy_setopt(p_curl, CURLOPT_OBSOLETE40, (0L)); /* OBSOLETE, do not use! */

		curl_easy_setopt(p_curl, CURLOPT_VERBOSE, (1L));      /* talk a lot */
		curl_easy_setopt(p_curl, CURLOPT_HEADER, (1L));       /* throw the header out too */
		curl_easy_setopt(p_curl, CURLOPT_NOPROGRESS, (0L));   /* shut off the progress meter */
		curl_easy_setopt(p_curl, CURLOPT_NOBODY, (1L));       /* use HEAD to get http document */
		curl_easy_setopt(p_curl, CURLOPT_FAILONERROR, (1L));  /* no output on http error codes >= 400 */
		curl_easy_setopt(p_curl, CURLOPT_UPLOAD, (0L));       /* this is an upload */
		curl_easy_setopt(p_curl, CURLOPT_POST, (0L));         /* HTTP POST method */
		curl_easy_setopt(p_curl, CURLOPT_DIRLISTONLY, (1L));  /* bare names when listing directories */

		curl_easy_setopt(p_curl, CURLOPT_APPEND, (1L));       /* Append instead of overwrite on upload! */

		/* Specify whether to read the user+password from the .netrc or the URL.
		* This must be one of the CURL_NETRC_* enums below. */
		curl_easy_setopt(p_curl, CURLOPT_NETRC, CURL_NETRC_IGNORED);

		curl_easy_setopt(p_curl, CURLOPT_FOLLOWLOCATION, (1L));  /* use Location: Luke! */

		curl_easy_setopt(p_curl, CURLOPT_TRANSFERTEXT, (1L)); /* transfer data in text/ASCII format */
		curl_easy_setopt(p_curl, CURLOPT_PUT, (0L));          /* HTTP PUT */

		//#if LIBCURL_VERSION >= 0x072000
		/* 55 = OBSOLETE */

		/* DEPRECATED
		* Function that will be called instead of the internal progress display
		* function. This function should be defined as the curl_progress_callback
		* prototype defines. */
		//curl_easy_setopt(p_curl, CURLOPT_PROGRESSFUNCTION, [](void *pclient, double dltotal, double dlnow, double ultotal, double ulnow){});

		/* Data passed to the CURLOPT_PROGRESSFUNCTION and CURLOPT_XFERINFOFUNCTION
		callbacks */
		//curl_easy_setopt(p_curl, CURLOPT_PROGRESSDATA, (void*)0L/*pclient*/);

		/* We want the referrer field set automatically when following locations */
		curl_easy_setopt(p_curl, CURLOPT_AUTOREFERER, (1L));

		/* Port of the proxy, can be set in the proxy string as well with:
		"[host]:[port]" */
		curl_easy_setopt(p_curl, CURLOPT_PROXYPORT, (""));

		/* size of the POST input data, if strlen() is not good to use */
		curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE, (-1L));

		/* tunnel non-http operations through a HTTP proxy */
		curl_easy_setopt(p_curl, CURLOPT_HTTPPROXYTUNNEL, (1L));

		/* Set the interface string to use as outgoing network interface */
		curl_easy_setopt(p_curl, CURLOPT_INTERFACE, ("")/*("eth0")*/);

		/* Set the krb4/5 security level, this also enables krb4/5 awareness.  This
		* is a string, 'clear', 'safe', 'confidential' or 'private'.  If the string
		* is set but doesn't match one of these, 'private' will be used.  */
		curl_easy_setopt(p_curl, CURLOPT_KRBLEVEL, (""));

		/* Set if we should verify the peer in ssl handshake, set 1 to verify. */
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, (0L));

		/* The CApath or CAfile used to validate the peer certificate
		this option is used only if SSL_VERIFYPEER is true */
		curl_easy_setopt(p_curl, CURLOPT_CAINFO, (""));

		/* 66 = OBSOLETE */
		/* 67 = OBSOLETE */

		/* Maximum number of http redirects to follow */
		curl_easy_setopt(p_curl, CURLOPT_MAXREDIRS, (3L));

		/* Pass a long set to 1 to get the date of the requested document (if
		possible)! Pass a zero to shut it off. */
		curl_easy_setopt(p_curl, CURLOPT_FILETIME, (1L));

		/* This points to a linked list of telnet options */
		curl_easy_setopt(p_curl, CURLOPT_TELNETOPTIONS, (struct curl_slist*)0L);

		/* Max amount of cached alive connections */
		curl_easy_setopt(p_curl, CURLOPT_MAXCONNECTS, (3L));

		//curl_easy_setopt(p_curl, CURLOPT_OBSOLETE72, (-1L)); /* OBSOLETE, do not use! */

		/* 73 = OBSOLETE */

		/* Set to explicitly use a new connection for the upcoming transfer.
		Do not use this unless you're absolutely sure of this, as it makes the
		operation slower and is less friendly for the network. */
		curl_easy_setopt(p_curl, CURLOPT_FRESH_CONNECT, (0L));

		/* Set to explicitly forbid the upcoming transfer's connection to be re-used
		when done. Do not use this unless you're absolutely sure of this, as it
		makes the operation slower and is less friendly for the network. */
		curl_easy_setopt(p_curl, CURLOPT_FORBID_REUSE, (0L));

		/* Set to a file name that contains random data for libcurl to use to
		seed the random engine when doing SSL connects. */
		curl_easy_setopt(p_curl, CURLOPT_RANDOM_FILE, (""));

		/* Set to the Entropy Gathering Daemon socket pathname */
		curl_easy_setopt(p_curl, CURLOPT_EGDSOCKET, (""));

		/* Time-out connect operations after this amount of seconds, if connects are
		OK within this time, then fine... This only aborts the connect phase. */
		curl_easy_setopt(p_curl, CURLOPT_CONNECTTIMEOUT, (0L));

		/* Function that will be called to store headers (instead of fwrite). The
		* parameters will use fwrite() syntax, make sure to follow them. */
		//curl_easy_setopt(p_curl, CURLOPT_HEADERFUNCTION, [](void * p_data, size_t n_size, size_t n_menb, void * p_argv){});
		curl_easy_setopt(p_curl, CURLOPT_HEADERFUNCTION, header_callback);

		/* Set this to force the HTTP request to get back to GET. Only really usable
		if POST, PUT or a custom request have been used first.
		*/
		curl_easy_setopt(p_curl, CURLOPT_HTTPGET, (0L));

		/* Set if we should verify the Common name from the peer certificate in ssl
		* handshake, set 1 to check existence, 2 to ensure that it matches the
		* provided hostname. */
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYHOST, (2L));

		/* Specify which file name to write all known cookies in after completed
		operation. Set file name to "-" (dash) to make it go to stdout. */
		curl_easy_setopt(p_curl, CURLOPT_COOKIEJAR, ("-"));

		/* Specify which SSL ciphers to use */
		curl_easy_setopt(p_curl, CURLOPT_SSL_CIPHER_LIST, (""));

		/* Specify which HTTP version to use! This must be set to one of the
		CURL_HTTP_VERSION* enums set below. */
		curl_easy_setopt(p_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);

		/* Specifically switch on or off the FTP engine's use of the EPSV command. By
		default, that one will always be attempted before the more traditional
		PASV command. */
		curl_easy_setopt(p_curl, CURLOPT_FTP_USE_EPSV, (0L));

		/* type of the file keeping your SSL-certificate ("DER", "PEM", "ENG") */
		curl_easy_setopt(p_curl, CURLOPT_SSLCERTTYPE, (""));

		/* name of the file keeping your private SSL-key */
		curl_easy_setopt(p_curl, CURLOPT_SSLKEY, (""));

		/* type of the file keeping your private SSL-key ("DER", "PEM", "ENG") */
		curl_easy_setopt(p_curl, CURLOPT_SSLKEYTYPE, (""));

		/* crypto engine for the SSL-sub system */
		curl_easy_setopt(p_curl, CURLOPT_SSLENGINE, (""));

		/* set the crypto engine for the SSL-sub system as default
		the param has no meaning...
		*/
		curl_easy_setopt(p_curl, CURLOPT_SSLENGINE_DEFAULT, (0L));

		/* Non-zero value means to use the global dns cache */
		//curl_easy_setopt(p_curl, CURLOPT_DNS_USE_GLOBAL_CACHE, (0L)); /* DEPRECATED, do not use! */

		/* DNS cache timeout */
		curl_easy_setopt(p_curl, CURLOPT_DNS_CACHE_TIMEOUT, (2L));

		/* send linked-list of pre-transfer QUOTE commands */
		curl_easy_setopt(p_curl, CURLOPT_PREQUOTE, (struct curl_slist*)0L);

		/* set the debug function */
		//curl_easy_setopt(p_curl, CURLOPT_DEBUGFUNCTION, [](CURL*p_curl, curl_infotype info_type, char *pdata, size_t n_size, void *p_argv){});
		curl_easy_setopt(p_curl, CURLOPT_DEBUGFUNCTION, debug_callback);

		/* set the data for the debug function */
		DEBUGBLOCK db = { 0x01 };
		curl_easy_setopt(p_curl, CURLOPT_DEBUGDATA, (DEBUGBLOCK*)&db);

		/* mark this as start of a cookie session */
		curl_easy_setopt(p_curl, CURLOPT_COOKIESESSION, (1L));

		/* The CApath directory used to validate the peer certificate
		this option is used only if SSL_VERIFYPEER is true */
		curl_easy_setopt(p_curl, CURLOPT_CAPATH, (""));

		/* Instruct libcurl to use a smaller receive buffer */
		curl_easy_setopt(p_curl, CURLOPT_BUFFERSIZE, (-1L));

		/* Instruct libcurl to not use any signal/alarm handlers, even when using
		timeouts. This option is useful for multi-threaded applications.
		See libcurl-the-guide for more background information. */
		curl_easy_setopt(p_curl, CURLOPT_NOSIGNAL, (1L));

		/* Provide a CURLShare for mutexing non-ts data */
		CURLSH *p_curlsh = curl_share_init();
		curl_share_setopt(p_curlsh, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
		curl_easy_setopt(p_curl, CURLOPT_SHARE, p_curlsh);
		curl_share_cleanup(p_curlsh);
		p_curlsh = 0;

		/* indicates type of proxy. accepted values are CURLPROXY_HTTP (default),
		CURLPROXY_HTTPS, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A and
		CURLPROXY_SOCKS5. */
		curl_easy_setopt(p_curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);

		/* Set the Accept-Encoding string. Use this to tell a server you would like
		the response to be compressed. Before 7.21.6, this was known as
		CURLOPT_ENCODING */
		curl_easy_setopt(p_curl, CURLOPT_ACCEPT_ENCODING, (""));

		/* Set pointer to private data */
		//struct private secrets;
		//curl_easy_setopt(p_curl, CURLOPT_PRIVATE, &secrets);

		/* Set aliases for HTTP 200 in the HTTP Response header */
		curl_easy_setopt(p_curl, CURLOPT_HTTP200ALIASES, (struct curl_slist*)0L);

		/* Continue to send authentication (user+password) when following locations,
		even when hostname changed. This can potentially send off the name
		and password to whatever host the server decides. */
		curl_easy_setopt(p_curl, CURLOPT_UNRESTRICTED_AUTH, (1L));

		/* Specifically switch on or off the FTP engine's use of the EPRT command (
		it also disables the LPRT attempt). By default, those ones will always be
		attempted before the good old traditional PORT command. */
		curl_easy_setopt(p_curl, CURLOPT_FTP_USE_EPRT, (0L));

		/* Set this to a bitmask value to enable the particular authentications
		methods you like. Use this in combination with CURLOPT_USERPWD.
		Note that setting multiple bits may cause extra network round-trips. */
		curl_easy_setopt(p_curl, CURLOPT_HTTPAUTH, CURLAUTH_NONE);

		/* Set the ssl context callback function, currently only for OpenSSL ssl_ctx
		in second argument. The function must be matching the
		curl_ssl_ctx_callback proto. */
		curl_easy_setopt(p_curl, CURLOPT_SSL_CTX_FUNCTION, sslctx_callback);

		/* Set the userdata for the ssl context callback function's third
		argument */
		//char *p_sslctx_data = /* example CA cert PEM - shortened */
		//	"-----BEGIN CERTIFICATE-----\n"
		//	"MIIHPTCCBSWgAwIBAgIBADANBgkqhkiG9w0BAQQFADB5MRAwDgYDVQQKEwdSb290\n"
		//	"IENBMR4wHAYDVQQLExVodHRwOi8vd3d3LmNhY2VydC5vcmcxIjAgBgNVBAMTGUNB\n"
		//	"IENlcnQgU2lnbmluZyBBdXRob3JpdHkxITAfBgkqhkiG9w0BCQEWEnN1cHBvcnRA\n"
		//	"Y2FjZXJ0Lm9yZzAeFw0wMzAzMzAxMjI5NDlaFw0zMzAzMjkxMjI5NDlaMHkxEDAO\n"
		//	"GCSNe9FINSkYQKyTYOGWhlC0elnYjyELn8+CkcY7v2vcB5G5l1YjqrZslMZIBjzk\n"
		//	"zk6q5PYvCdxTby78dOs6Y5nCpqyJvKeyRKANihDjbPIky/qbn3BHLt4Ui9SyIAmW\n"
		//	"omTxJBzcoTWcFbLUvFUufQb1nA5V9FrWk9p2rSVzTMVD\n"
		//	"-----END CERTIFICATE-----\n";
		curl_easy_setopt(p_curl, CURLOPT_SSL_CTX_DATA, (""));

		/* FTP Option that causes missing dirs to be created on the remote server.
		In 7.19.4 we introduced the convenience enums for this option using the
		CURLFTP_CREATE_DIR prefix.
		*/
		curl_easy_setopt(p_curl, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_NONE);

		/* Set this to a bitmask value to enable the particular authentications
		methods you like. Use this in combination with CURLOPT_PROXYUSERPWD.
		Note that setting multiple bits may cause extra network round-trips. */
		curl_easy_setopt(p_curl, CURLOPT_PROXYAUTH, CURLAUTH_NONE);

		/* FTP option that changes the timeout, in seconds, associated with
		getting a response.  This is different from transfer timeout time and
		essentially places a demand on the FTP server to acknowledge commands
		in a timely manner. */
		curl_easy_setopt(p_curl, CURLOPT_FTP_RESPONSE_TIMEOUT, (0L));

		/* Set this option to one of the CURL_IPRESOLVE_* defines (see below) to
		tell libcurl to resolve names to those IP versions only. This only has
		affect on systems with support for more than one, i.e IPv4 _and_ IPv6. */
		curl_easy_setopt(p_curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);

		/* Set this option to limit the size of a file that will be downloaded from
		an HTTP or FTP server.

		Note there is also _LARGE version which adds large file support for
		platforms which have larger off_t sizes.  See MAXFILESIZE_LARGE below. */
		curl_easy_setopt(p_curl, CURLOPT_MAXFILESIZE, (-1LL));

		/* See the comment for INFILESIZE above, but in short, specifies
		* the size of the file being uploaded.  -1 means unknown.
		*/
		curl_easy_setopt(p_curl, CURLOPT_INFILESIZE_LARGE, (-1LL));

		/* Sets the continuation offset.  There is also a LONG version of this;
		* look above for RESUME_FROM.
		*/
		curl_easy_setopt(p_curl, CURLOPT_RESUME_FROM_LARGE, (-1LL));

		/* Sets the maximum size of data that will be downloaded from
		* an HTTP or FTP server.  See MAXFILESIZE above for the LONG version.
		*/
		curl_easy_setopt(p_curl, CURLOPT_MAXFILESIZE_LARGE, (-1LL));

		/* Set this option to the file name of your .netrc file you want libcurl
		to parse (using the CURLOPT_NETRC option). If not set, libcurl will do
		a poor attempt to find the user's home directory and check for a .netrc
		file in there. */
		curl_easy_setopt(p_curl, CURLOPT_NETRC_FILE, (""));

		/* Enable SSL/TLS for FTP, pick one of:
		CURLUSESSL_TRY     - try using SSL, proceed anyway otherwise
		CURLUSESSL_CONTROL - SSL for the control connection or fail
		CURLUSESSL_ALL     - SSL for all communication or fail
		*/
		curl_easy_setopt(p_curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);

		/* The _LARGE version of the standard POSTFIELDSIZE option */
		curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE_LARGE, (-1LL));

		/* Enable/disable the TCP Nagle algorithm */
		curl_easy_setopt(p_curl, CURLOPT_TCP_NODELAY, (0L));

		/* 122 OBSOLETE, used in 7.12.3. Gone in 7.13.0 */
		/* 123 OBSOLETE. Gone in 7.16.0 */
		/* 124 OBSOLETE, used in 7.12.3. Gone in 7.13.0 */
		/* 125 OBSOLETE, used in 7.12.3. Gone in 7.13.0 */
		/* 126 OBSOLETE, used in 7.12.3. Gone in 7.13.0 */
		/* 127 OBSOLETE. Gone in 7.16.0 */
		/* 128 OBSOLETE. Gone in 7.16.0 */

		/* When FTP over SSL/TLS is selected (with CURLOPT_USE_SSL), this option
		can be used to change libcurl's default action which is to first try
		"AUTH SSL" and then "AUTH TLS" in this order, and proceed when a OK
		response has been received.

		Available parameters are:
		CURLFTPAUTH_DEFAULT - let libcurl decide
		CURLFTPAUTH_SSL     - try "AUTH SSL" first, then TLS
		CURLFTPAUTH_TLS     - try "AUTH TLS" first, then SSL
		*/
		curl_easy_setopt(p_curl, CURLOPT_FTPSSLAUTH, CURLFTPAUTH_DEFAULT);

		curl_easy_setopt(p_curl, CURLOPT_IOCTLFUNCTION, ioctl_callback);
		curl_easy_setopt(p_curl, CURLOPT_IOCTLDATA, (CALLBACKDATA*)0L);

		/* 132 OBSOLETE. Gone in 7.16.0 */
		/* 133 OBSOLETE. Gone in 7.16.0 */

		/* zero terminated string for pass on to the FTP server when asked for
		"account" info */
		curl_easy_setopt(p_curl, CURLOPT_FTP_ACCOUNT, (""));

		/* feed cookie into cookie engine */
		curl_easy_setopt(p_curl, CURLOPT_COOKIELIST, (""));

		/* ignore Content-Length */
		curl_easy_setopt(p_curl, CURLOPT_IGNORE_CONTENT_LENGTH, (1L));

		/* Set to non-zero to skip the IP address received in a 227 PASV FTP server
		response. Typically used for FTP-SSL purposes but is not restricted to
		that. libcurl will then instead use the same IP address it used for the
		control connection. */
		curl_easy_setopt(p_curl, CURLOPT_FTP_SKIP_PASV_IP, (1L));

		/* Select "file method" to use when doing FTP, see the curl_ftpmethod
		above. */
		curl_easy_setopt(p_curl, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_DEFAULT);

		/* Local port number to bind the socket to */
		curl_easy_setopt(p_curl, CURLOPT_LOCALPORT, (-1L));

		/* Number of ports to try, including the first one set with LOCALPORT.
		Thus, setting it to 1 will make no additional attempts but the first.
		*/
		curl_easy_setopt(p_curl, CURLOPT_LOCALPORTRANGE, (-1L));

		/* no transfer, set up connection and let application use the socket by
		extracting it with CURLINFO_LASTSOCKET */
		curl_easy_setopt(p_curl, CURLOPT_CONNECT_ONLY, (0L));

		/* Function that will be called to convert from the
		network encoding (instead of using the iconv calls in libcurl) */

		curl_easy_setopt(p_curl, CURLOPT_CONV_FROM_NETWORK_FUNCTION, conv_from_network_callback);

		/* Function that will be called to convert to the
		network encoding (instead of using the iconv calls in libcurl) */
		curl_easy_setopt(p_curl, CURLOPT_CONV_TO_NETWORK_FUNCTION, conv_to_network_callback);

		/* Function that will be called to convert from UTF8
		(instead of using the iconv calls in libcurl)
		Note that this is used only for SSL certificate processing */
		curl_easy_setopt(p_curl, CURLOPT_CONV_FROM_UTF8_FUNCTION, conv_from_utf8_callback);

		/* if the connection proceeds too quickly then need to slow it down */
		/* limit-rate: maximum number of bytes per second to send or receive */
		curl_easy_setopt(p_curl, CURLOPT_MAX_SEND_SPEED_LARGE, (-1LL));
		curl_easy_setopt(p_curl, CURLOPT_MAX_RECV_SPEED_LARGE, (-1LL));

		/* Pointer to command string to send if USER/PASS fails. */
		curl_easy_setopt(p_curl, CURLOPT_FTP_ALTERNATIVE_TO_USER, (""));

		/* callback function for setting socket options */
		curl_easy_setopt(p_curl, CURLOPT_SOCKOPTFUNCTION, sockopt_callback);
		curl_easy_setopt(p_curl, CURLOPT_SOCKOPTDATA, (-1L));

		/* set to 0 to disable session ID re-use for this transfer, default is
		enabled (== 1) */
		curl_easy_setopt(p_curl, CURLOPT_SSL_SESSIONID_CACHE, (0L));

		/* allowed SSH authentication methods */
		curl_easy_setopt(p_curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_NONE);

		/* Used by scp/sftp to do public/private key authentication */
		curl_easy_setopt(p_curl, CURLOPT_SSH_PUBLIC_KEYFILE, (""));
		curl_easy_setopt(p_curl, CURLOPT_SSH_PRIVATE_KEYFILE, (""));

		/* Send CCC (Clear Command Channel) after authentication */
		curl_easy_setopt(p_curl, CURLOPT_FTP_SSL_CCC, CURLFTPSSL_CCC_NONE);

		/* Same as TIMEOUT and CONNECTTIMEOUT, but with ms resolution */
		curl_easy_setopt(p_curl, CURLOPT_TIMEOUT_MS, (0L));
		curl_easy_setopt(p_curl, CURLOPT_CONNECTTIMEOUT_MS, (0L));

		/* set to zero to disable the libcurl's decoding and thus pass the raw body
		data to the application even when it is encoded/compressed */
		curl_easy_setopt(p_curl, CURLOPT_HTTP_TRANSFER_DECODING, (0L));
		curl_easy_setopt(p_curl, CURLOPT_HTTP_CONTENT_DECODING, (0L));

		/* Permission used when creating new files and directories on the remote
		server for protocols that support it, SFTP/SCP/FILE */
		curl_easy_setopt(p_curl, CURLOPT_NEW_FILE_PERMS, (0000L));
		curl_easy_setopt(p_curl, CURLOPT_NEW_DIRECTORY_PERMS, (0000L));

		/* Set the behaviour of POST when redirecting. Values must be set to one
		of CURL_REDIR* defines below. This used to be called CURLOPT_POST301 */
		curl_easy_setopt(p_curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);

		/* used by scp/sftp to verify the host's public key */
		curl_easy_setopt(p_curl, CURLOPT_SSH_HOST_PUBLIC_KEY_MD5, (""));

		/* Callback function for opening socket (instead of socket(2)). Optionally,
		callback is able change the address or refuse to connect returning
		CURL_SOCKET_BAD.  The callback should have type
		curl_opensocket_callback */
		curl_easy_setopt(p_curl, CURLOPT_OPENSOCKETFUNCTION, opensocket_callback);
		curl_easy_setopt(p_curl, CURLOPT_OPENSOCKETDATA, (curl_socket_t*)0L);

		/* POST volatile input fields. */
		char local_buffer[MAXWORD] = { 0 };
		curl_easy_setopt(p_curl, CURLOPT_COPYPOSTFIELDS, local_buffer);

		/* set transfer mode (;type=<a|i>) when doing FTP via an HTTP proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_TRANSFER_MODE, (0L));

		/* Callback function for seeking in the input stream */
		curl_easy_setopt(p_curl, CURLOPT_SEEKFUNCTION, seek_callback);
		curl_easy_setopt(p_curl, CURLOPT_SEEKDATA, (CALLBACKDATA*)0L);

		/* CRL file */
		curl_easy_setopt(p_curl, CURLOPT_CRLFILE, (""));

		/* Issuer certificate */
		curl_easy_setopt(p_curl, CURLOPT_ISSUERCERT, (""));

		/* (IPv6) Address scope */
		/*
		* 0x2 link-local
		* 0x5 site-local
		* 0x8 organization-local
		* 0xe global ...
		*/
		curl_easy_setopt(p_curl, CURLOPT_ADDRESS_SCOPE, (-1L));

		/* Collect certificate chain info and allow it to get retrievable with
		CURLINFO_CERTINFO after the transfer is complete. */
		curl_easy_setopt(p_curl, CURLOPT_CERTINFO, (0L));

		/* "name" and "pwd" to use when fetching. */
		curl_easy_setopt(p_curl, CURLOPT_USERNAME, (""));
		curl_easy_setopt(p_curl, CURLOPT_PASSWORD, (""));

		/* "name" and "pwd" to use with Proxy when fetching. */
		curl_easy_setopt(p_curl, CURLOPT_PROXYUSERNAME, (""));
		curl_easy_setopt(p_curl, CURLOPT_PROXYPASSWORD, (""));

		/* Comma separated list of hostnames defining no-proxy zones. These should
		match both hostnames directly, and hostnames within a domain. For
		example, local.com will match local.com and www.local.com, but NOT
		notlocal.com or www.notlocal.com. For compatibility with other
		implementations of this, .local.com will be considered to be the same as
		local.com. A single * is the only valid wildcard, and effectively
		disables the use of proxy. */
		curl_easy_setopt(p_curl, CURLOPT_NOPROXY, (""));

		/* block size for TFTP transfers */
		curl_easy_setopt(p_curl, CURLOPT_TFTP_BLKSIZE, (-1L));

		/* Socks Service */
		//curl_easy_setopt(p_curl, CURLOPT_SOCKS5_GSSAPI_SERVICE, ("")); /* DEPRECATED, do not use! */

		/* Socks Service */
		curl_easy_setopt(p_curl, CURLOPT_SOCKS5_GSSAPI_NEC, (0L));

		/* set the bitmask for the protocols that are allowed to be used for the
		transfer, which thus helps the app which takes URLs from users or other
		external inputs and want to restrict what protocol(s) to deal
		with. Defaults to CURLPROTO_ALL. */
		curl_easy_setopt(p_curl, CURLOPT_PROTOCOLS, CURLPROTO_ALL);

		/* set the bitmask for the protocols that libcurl is allowed to follow to,
		as a subset of the CURLOPT_PROTOCOLS ones. That means the protocol needs
		to be set in both bitmasks to be allowed to get redirected to. Defaults
		to all protocols except FILE and SCP. */
		curl_easy_setopt(p_curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_ALL);

		/* set the SSH knownhost file name to use */
		curl_easy_setopt(p_curl, CURLOPT_SSH_KNOWNHOSTS, (""));

		/* set the SSH host key callback, must point to a curl_sshkeycallback
		function */
		curl_easy_setopt(p_curl, CURLOPT_SSH_KEYFUNCTION, ssh_key_callback);

		/* set the SSH host key callback custom pointer */
		curl_easy_setopt(p_curl, CURLOPT_SSH_KEYDATA, (CALLBACKDATA*)0L);

		/* set the SMTP mail originator */
		curl_easy_setopt(p_curl, CURLOPT_MAIL_FROM, (""));

		/* set the list of SMTP mail receiver(s) */
		curl_easy_setopt(p_curl, CURLOPT_MAIL_RCPT, (struct curl_slist*)0L);

		/* FTP: send PRET before PASV */
		curl_easy_setopt(p_curl, CURLOPT_FTP_USE_PRET, (0L));

		/* RTSP request method (OPTIONS, SETUP, PLAY, etc...) */
		curl_easy_setopt(p_curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_NONE);

		/* The RTSP session identifier */
		curl_easy_setopt(p_curl, CURLOPT_RTSP_SESSION_ID, (""));

		/* The RTSP stream URI */
		curl_easy_setopt(p_curl, CURLOPT_RTSP_STREAM_URI, (""));

		/* The Transport: header to use in RTSP requests */
		curl_easy_setopt(p_curl, CURLOPT_RTSP_TRANSPORT, (""));

		/* Manually initialize the client RTSP CSeq for this handle */
		curl_easy_setopt(p_curl, CURLOPT_RTSP_CLIENT_CSEQ, (-1L));

		/* Manually initialize the server RTSP CSeq for this handle */
		curl_easy_setopt(p_curl, CURLOPT_RTSP_SERVER_CSEQ, (-1L));

		/* The stream to pass to INTERLEAVEFUNCTION. */
		curl_easy_setopt(p_curl, CURLOPT_INTERLEAVEDATA, (CALLBACKDATA*)0L);

		/* Let the application define a custom write method for RTP data */
		curl_easy_setopt(p_curl, CURLOPT_INTERLEAVEFUNCTION, interleavedata_callback);

		/* Turn on wildcard matching */
		curl_easy_setopt(p_curl, CURLOPT_WILDCARDMATCH, (0L));

		/* Directory matching callback called before downloading of an
		individual file (chunk) started */
		curl_easy_setopt(p_curl, CURLOPT_CHUNK_BGN_FUNCTION, chunk_bgn_callback);

		/* Directory matching callback called after the file (chunk)
		was downloaded, or skipped */
		curl_easy_setopt(p_curl, CURLOPT_CHUNK_END_FUNCTION, chunk_end_callback);

		/* Change match (fnmatch-like) callback for wildcard matching */
		curl_easy_setopt(p_curl, CURLOPT_FNMATCH_FUNCTION, fnmatch_callback);

		/* Let the application define custom chunk data pointer */
		curl_easy_setopt(p_curl, CURLOPT_CHUNK_DATA, (CALLBACKDATA*)0L);

		/* FNMATCH_FUNCTION user pointer */
		curl_easy_setopt(p_curl, CURLOPT_FNMATCH_DATA, (CALLBACKDATA*)0L);

		/* send linked-list of name:port:address sets */
		curl_easy_setopt(p_curl, CURLOPT_RESOLVE, (struct curl_slist*)0L);

		/* Set a username for authenticated TLS */
		curl_easy_setopt(p_curl, CURLOPT_TLSAUTH_USERNAME, (""));

		/* Set a password for authenticated TLS */
		curl_easy_setopt(p_curl, CURLOPT_TLSAUTH_PASSWORD, (""));

		/* Set authentication type for authenticated TLS */
		curl_easy_setopt(p_curl, CURLOPT_TLSAUTH_TYPE, (""));

		/* Set to 1 to enable the "TE:" header in HTTP requests to ask for
		compressed transfer-encoded responses. Set to 0 to disable the use of TE:
		in outgoing requests. The current default is 0, but it might change in a
		future libcurl release.

		libcurl will ask for the compressed methods it knows of, and if that
		isn't any, it will not ask for transfer-encoding at all even if this
		option is set to 1.

		*/
		curl_easy_setopt(p_curl, CURLOPT_TRANSFER_ENCODING, (1L));

		/* Callback function for closing socket (instead of close(2)). The callback
		should have type curl_closesocket_callback */
		curl_easy_setopt(p_curl, CURLOPT_CLOSESOCKETFUNCTION, closesocket_callback);
		curl_easy_setopt(p_curl, CURLOPT_CLOSESOCKETDATA, (curl_socket_t*)0L);

		/* allow GSSAPI credential delegation */
		curl_easy_setopt(p_curl, CURLOPT_GSSAPI_DELEGATION, CURLGSSAPI_DELEGATION_NONE);

		/* Set the name servers to use for DNS resolution */
		curl_easy_setopt(p_curl, CURLOPT_DNS_SERVERS, (""));

		/* Time-out accept operations (currently for FTP only) after this amount
		of milliseconds. */
		curl_easy_setopt(p_curl, CURLOPT_ACCEPTTIMEOUT_MS, (0L));

		/* Set TCP keepalive */
		curl_easy_setopt(p_curl, CURLOPT_TCP_KEEPALIVE, (1L));

		/* non-universal keepalive knobs (Linux, AIX, HP-UX, more) */
		curl_easy_setopt(p_curl, CURLOPT_TCP_KEEPIDLE, (120L));
		curl_easy_setopt(p_curl, CURLOPT_TCP_KEEPINTVL, (60L));

		/* Enable/disable specific SSL features with a bitmask, see CURLSSLOPT_* */
		curl_easy_setopt(p_curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_ALLOW_BEAST | CURLSSLOPT_NO_REVOKE);

		/* Set the SMTP auth originator */
		curl_easy_setopt(p_curl, CURLOPT_MAIL_AUTH, (""));

		/* Enable/disable SASL initial response */
		curl_easy_setopt(p_curl, CURLOPT_SASL_IR, (1L));

		/* Function that will be called instead of the internal progress display
		* function. This function should be defined as the curl_xferinfo_callback
		* prototype defines. (Deprecates CURLOPT_PROGRESSFUNCTION) */
		curl_easy_setopt(p_curl, CURLOPT_XFERINFOFUNCTION, [](void *p_argv, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow){});

		/* The XOAUTH2 bearer token */
		curl_easy_setopt(p_curl, CURLOPT_XOAUTH2_BEARER, (""));

		/* Set the interface string to use as outgoing network
		* interface for DNS requests.
		* Only supported by the c-ares DNS backend */
		curl_easy_setopt(p_curl, CURLOPT_DNS_INTERFACE, (""));

		/* Set the local IPv4 address to use for outgoing DNS requests.
		* Only supported by the c-ares DNS backend */
		curl_easy_setopt(p_curl, CURLOPT_DNS_LOCAL_IP4, (""));

		/* Set the local IPv4 address to use for outgoing DNS requests.
		* Only supported by the c-ares DNS backend */
		curl_easy_setopt(p_curl, CURLOPT_DNS_LOCAL_IP6, (""));

		/* Set authentication options directly */
		curl_easy_setopt(p_curl, CURLOPT_LOGIN_OPTIONS, (""));

		/* Enable/disable TLS NPN extension (http2 over ssl might fail without) */
		curl_easy_setopt(p_curl, CURLOPT_SSL_ENABLE_NPN, (0L));

		/* Enable/disable TLS ALPN extension (http2 over ssl might fail without) */
		curl_easy_setopt(p_curl, CURLOPT_SSL_ENABLE_ALPN, (0L));

		/* Time to wait for a response to a HTTP request containing an
		* Expect: 100-continue header before sending the data anyway. */
		curl_easy_setopt(p_curl, CURLOPT_EXPECT_100_TIMEOUT_MS, (100L));

		/* This points to a linked list of headers used for proxy requests only,
		struct curl_slist kind */
		curl_easy_setopt(p_curl, CURLOPT_PROXYHEADER, (struct curl_slist*)0L);

		/* Pass in a bitmask of "header options" */
		curl_easy_setopt(p_curl, CURLOPT_HEADEROPT, CURLHEADER_UNIFIED);

		/* The public key in DER form used to validate the peer public key
		this option is used only if SSL_VERIFYPEER is true */
		curl_easy_setopt(p_curl, CURLOPT_PINNEDPUBLICKEY, (""));

		/* Path to Unix domain socket */
		curl_easy_setopt(p_curl, CURLOPT_UNIX_SOCKET_PATH, (""));

		/* Set if we should verify the certificate status. */
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYSTATUS, (1L));

		/* Set if we should enable TLS false start. */
		curl_easy_setopt(p_curl, CURLOPT_SSL_FALSESTART, (1L));

		/* Do not squash dot-dot sequences */
		curl_easy_setopt(p_curl, CURLOPT_PATH_AS_IS, (1L));

		/* Proxy Service Name */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_SERVICE_NAME, (""));

		/* Service Name */
		curl_easy_setopt(p_curl, CURLOPT_SERVICE_NAME, (""));

		/* Wait/don't wait for pipe/mutex to clarify */
		curl_easy_setopt(p_curl, CURLOPT_PIPEWAIT, (1L));

		/* Set the protocol used when curl is given a URL without a protocol */
		curl_easy_setopt(p_curl, CURLOPT_DEFAULT_PROTOCOL, (""));

		/* Set stream weight, 1 - 256 (default is 16) */
		curl_easy_setopt(p_curl, CURLOPT_STREAM_WEIGHT, (0L));

		/* Set stream dependency on another CURL handle */
		curl_easy_setopt(p_curl, CURLOPT_STREAM_DEPENDS, (CURL*)0L);

		/* Set E-xclusive stream dependency on another CURL handle */
		curl_easy_setopt(p_curl, CURLOPT_STREAM_DEPENDS_E, (CURL*)0L);

		/* Do not send any tftp option requests to the server */
		curl_easy_setopt(p_curl, CURLOPT_TFTP_NO_OPTIONS, (1L));

		/* Linked-list of host:port:connect-to-host:connect-to-port,
		overrides the URL's host:port (only for the network layer) */
		curl_easy_setopt(p_curl, CURLOPT_CONNECT_TO, (struct curl_slist *)0L);

		/* Set TCP Fast Open */
		curl_easy_setopt(p_curl, CURLOPT_TCP_FASTOPEN, (1L));

		/* Continue to send data if the server responds early with an
		* HTTP status code >= 300 */
		curl_easy_setopt(p_curl, CURLOPT_KEEP_SENDING_ON_ERROR, (1L));

		/* The CApath or CAfile used to validate the proxy certificate
		this option is used only if PROXY_SSL_VERIFYPEER is true */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_CAINFO, (""));

		/* The CApath directory used to validate the proxy certificate
		this option is used only if PROXY_SSL_VERIFYPEER is true */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_CAPATH, (""));

		/* Set if we should verify the proxy in ssl handshake,
		set 1 to verify. */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_SSL_VERIFYPEER, (0L));

		/* Set if we should verify the Common name from the proxy certificate in ssl
		* handshake, set 1 to check existence, 2 to ensure that it matches
		* the provided hostname. */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_SSL_VERIFYHOST, (2L));

		/* What version to specifically try to use for proxy.
		See CURL_SSLVERSION defines below. */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_SSLVERSION, CURL_SSLVERSION_DEFAULT);

		/* Set a username for authenticated TLS for proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_TLSAUTH_USERNAME, (""));

		/* Set a password for authenticated TLS for proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_TLSAUTH_PASSWORD, (""));

		/* Set authentication type for authenticated TLS for proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_TLSAUTH_TYPE, (""));

		/* name of the file keeping your private SSL-certificate for proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_SSLCERT, (""));

		/* type of the file keeping your SSL-certificate ("DER", "PEM", "ENG") for
		proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_SSLCERTTYPE, (""));

		/* name of the file keeping your private SSL-key for proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_SSLKEY, (""));

		/* type of the file keeping your private SSL-key ("DER", "PEM", "ENG") for
		proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_SSLKEYTYPE, (""));

		/* password for the SSL private key for proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_KEYPASSWD, (""));

		/* Specify which SSL ciphers to use for proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_SSL_CIPHER_LIST, (""));

		/* CRL file for proxy */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_CRLFILE, (""));

		/* Enable/disable specific SSL features with a bitmask for proxy, see
		CURLSSLOPT_* */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_SSL_OPTIONS, (""));

		/* Name of pre proxy to use. */
		curl_easy_setopt(p_curl, CURLOPT_PRE_PROXY, (""));

		/* The public key in DER form used to validate the proxy public key
		this option is used only if PROXY_SSL_VERIFYPEER is true */
		curl_easy_setopt(p_curl, CURLOPT_PROXY_PINNEDPUBLICKEY, (""));

		/* Path to an abstract Unix domain socket */
		curl_easy_setopt(p_curl, CURLOPT_ABSTRACT_UNIX_SOCKET, (""));

		/* Suppress proxy CONNECT response headers from user callbacks */
		curl_easy_setopt(p_curl, CURLOPT_SUPPRESS_CONNECT_HEADERS, (1L));

		/* The request target, instead of extracted from the URL */
		curl_easy_setopt(p_curl, CURLOPT_REQUEST_TARGET, (""));

		/* bitmask of allowed auth methods for connections to SOCKS5 proxies */
		curl_easy_setopt(p_curl, CURLOPT_SOCKS5_AUTH, CURLAUTH_NONE);
	}
	
	typedef int(*PFN_REQUEST_HANDLER)(CURL *, CALLBACKDATA *, CALLBACKDATA *, CALLBACKDATA *);
	typedef int(*PFN_CURLOPT_HANDLER)(CURL *);
	typedef int(*PFN_RESPONSE_HANDLER)(CURL *, CALLBACKDATA *, CALLBACKDATA *, CURLcode);

	__inline static int request_handler(CURL * pCurl, CALLBACKDATA *pCBDRequrl, CALLBACKDATA * pCBDHeader, CALLBACKDATA * pCBDPostFields)
	{
		int result = 0;
		//har * pRequrl = "X";
		//char * pPostFields = "X";
		//char * pHeader = "X";

		//pCBDRequrl->copy(pRequrl, strlen(pRequrl));
		//pCBDPostFields->copy(pPostFields, strlen(pPostFields));
		//pCBDHeader->copy(pHeader, strlen(pHeader));

		return result;
	}
	__inline static int curlopt_handler(CURL * p_curl)
	{
		int result = 0;

		curl_easy_setopt(p_curl, CURLOPT_FORBID_REUSE, (1L));
		curl_easy_setopt(p_curl, CURLOPT_NOSIGNAL, (1L));

		// send it verbose for max debuggaility
		curl_easy_setopt(p_curl, CURLOPT_VERBOSE, (1L));

		// set debug function callback
		curl_easy_setopt(p_curl, CURLOPT_DEBUGFUNCTION, (0L));
		
		// set accept encoding("'gzip,deflate'")
		curl_easy_setopt(p_curl, CURLOPT_ACCEPT_ENCODING, (""));

		// HTTP/2 please
		curl_easy_setopt(p_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

		// set timeout(one milliseconds)
		curl_easy_setopt(p_curl, CURLOPT_TIMEOUT_MS, (0L));
		curl_easy_setopt(p_curl, CURLOPT_TRANSFERTEXT, (1L));
		curl_easy_setopt(p_curl, CURLOPT_FOLLOWLOCATION, (1L));
		curl_easy_setopt(p_curl, CURLOPT_AUTOREFERER, (1L));

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, (0L));
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYHOST, (2L));
		curl_easy_setopt(p_curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);

		curl_easy_setopt(p_curl, CURLOPT_POST, (0L));

		return result;
	}

	__inline static int response_handler(CURL * pCurl, CALLBACKDATA *pCBDHeader, CALLBACKDATA *pCBDData, CURLcode cURLCode)
	{
		int result = cURLCode;

		if (cURLCode != CURLE_OK)
		{
			printf("response handler failed! error id=%d\n", cURLCode);
		}
		else
		{
			printf("[header]=[%s]\n[data]=[%s]\n", pCBDHeader->p, pCBDData->p);
		}

		return result;
	}

	//curl 回调处理返回数据函数
	__inline static size_t write_native_data_callback(void * p_data, size_t n_size, size_t n_menb, void * p_argv)
	{
		CALLBACKDATA * pcbd = (CALLBACKDATA*)p_argv;
		if (pcbd && p_data)
		{
			pcbd->append((char *)p_data, n_size * n_menb);
		}
		return n_size * n_menb;
	}
	//curl 回调处理返回数据函数
	__inline static size_t write_dynamic_data_callback(void * p_data, size_t n_size, size_t n_menb, void * p_argv)
	{
		std::string * pstr = dynamic_cast<std::string *>((std::string*)p_argv);
		if (pstr && p_data)
		{
			pstr->append((char *)p_data, n_size * n_menb);
		}

		return n_size * n_menb;
	}
	__inline static int curl_http_exec(PFN_REQUEST_HANDLER request_handler = 0, PFN_CURLOPT_HANDLER curlopt_handler = 0, PFN_RESPONSE_HANDLER response_handler = 0, CURL * pCurl = 0)
	{
		int result = (-1);
		CURL * p_curl = 0;
		CURLcode curl_code = CURLE_OK;
		struct curl_slist *p_curl_slist_header = 0;

		CALLBACKDATA * pcbd_req_url = 0;
		CALLBACKDATA * pcbd_req_header = 0;
		CALLBACKDATA * pcbd_req_postfields = 0;
		CALLBACKDATA * pcbd_resp_header = 0;
		CALLBACKDATA * pcbd_resp_data = 0;

		if (pCurl)
		{
			p_curl = pCurl;
		}
		else
		{
			p_curl = curl_easy_init();
		}

		if (p_curl)
		{
			pcbd_req_url = (CALLBACKDATA *)CALLBACKDATA::startup();
			pcbd_req_header = (CALLBACKDATA *)CALLBACKDATA::startup();
			pcbd_req_postfields = (CALLBACKDATA *)CALLBACKDATA::startup();
			pcbd_resp_header = (CALLBACKDATA *)CALLBACKDATA::startup();
			pcbd_resp_data = (CALLBACKDATA *)CALLBACKDATA::startup();

			// write header and body data		
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, write_native_data_callback);
			curl_easy_setopt(p_curl, CURLOPT_WRITEHEADER, (void *)pcbd_resp_header);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, (void *)pcbd_resp_data);

			// write header and body data
			//std::string str_header = "";
			//std::string str_data = "";
			//curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, write_dynamic_data);
			//curl_easy_setopt(p_curl, CURLOPT_WRITEHEADER, (void *)&str_header);
			//curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, (void *)&str_data);

			if (request_handler)
			{
				result = request_handler(p_curl, pcbd_req_url, pcbd_req_header, pcbd_req_postfields);
			}

			// set http header list data
			p_curl_slist_header = curl_slist_append(p_curl_slist_header, pcbd_req_header->p);
			curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, p_curl_slist_header);

			// set request url
			curl_easy_setopt(p_curl, CURLOPT_URL, pcbd_req_url->p);

			// post data
			curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, pcbd_req_postfields->p);


			if (curlopt_handler)
			{
				curlopt_handler(p_curl);
			}

			result = curl_code = curl_easy_perform(p_curl);

			if (response_handler)
			{
				result = response_handler(p_curl, pcbd_resp_header, pcbd_resp_data, curl_code);
			}

			if (pcbd_resp_data)
			{
				pcbd_resp_data->cleanup();
			}
			if (pcbd_resp_header)
			{
				pcbd_resp_header->cleanup();
			}

			if (pcbd_req_postfields)
			{
				pcbd_req_postfields->cleanup();
			}
			if (pcbd_req_header)
			{
				pcbd_req_header->cleanup();
			}
			if (pcbd_req_url)
			{
				pcbd_req_url->cleanup();
			}

			if (p_curl_slist_header)
			{
				curl_slist_free_all(p_curl_slist_header);
				p_curl_slist_header = 0;
			}
			if (!pCurl)
			{
				curl_easy_cleanup(p_curl);
				p_curl = 0;
			}
		}

		return result;
	}

	__inline static int http_post_form(void)
	{
		CURL *curl;

		CURLM *multi_handle;
		int still_running;

		struct curl_httppost *formpost = NULL;
		struct curl_httppost *lastptr = NULL;
		struct curl_slist *headerlist = NULL;
		static const char buf[] = "Expect:";
		//static const char buf[] = "Content-Type: multipart/form-data";
		std::string strFileData;
		FILE * pF = fopen(std::string("D:\\123.png").c_str(), "rb");
		//std::string strError = strerror(errno);
		if (pF)
		{
			fseek(pF, 0, SEEK_END);
			size_t len = ftell(pF);
			if (len)
			{
				fseek(pF, 0, SEEK_SET);
				strFileData.resize(len, '\0');
				fread((void *)strFileData.c_str(), strFileData.size(), 1, pF);
			}
			fclose(pF);
		}

		std::string strMsgData = "{\"id\":123}";
		// set up the header
		curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "cache-control",
			CURLFORM_COPYCONTENTS, "no-cache",
			CURLFORM_END);
		curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "msg",
			CURLFORM_COPYCONTENTS, strMsgData.c_str(),
			CURLFORM_END);
		curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "content-type",
			CURLFORM_COPYCONTENTS, "multipart/form-data",
			CURLFORM_END);

		curl_formadd(&formpost, &lastptr,
			CURLFORM_COPYNAME, "file",  // <--- the (in this case) wanted file-Tag!
			CURLFORM_BUFFER, "data",
			CURLFORM_BUFFERPTR, strFileData.data(),
			CURLFORM_BUFFERLENGTH, strFileData.size(),
			CURLFORM_END);

		curl = curl_easy_init();

		/* initialize custom header list (stating that Expect: 100-continue is not
		wanted */
		headerlist = curl_slist_append(headerlist, buf);
		if (curl) {

			/* what URL that receives this POST */
			curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.246:8099/ZSWXApi/api/ZSWX/UploadFile");
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
			curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

			CURLcode res = curl_easy_perform(curl);
			/* Check for errors */
			if (res != CURLE_OK)
			{
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
			}

			/* always cleanup */
			curl_easy_cleanup(curl);

			/* then cleanup the formpost chain */
			curl_formfree(formpost);

			/* free slist */
			curl_slist_free_all(headerlist);
		}
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////
//函数功能:传入URL和要保存的文件名称下载文件
//函数参数:
//		strSavePathFileName		要保存的文件路径名
//		strRequestURL			下载地址URL
//		strHeaderData			要发送的头部数据字符串数组(\r\n为分隔符)
//		strPostFields			发送的POST域数据
//		bVerbose				是否为详细日志信息
//		nDelayTime				超时设置，默认为60000毫秒
//返回值:
//		0, 成功
//		-1,curl访问URL失败
//		-2,curl初始化失败
//		-3,创建或打开文件失败
int curl_http_get_file(string strSavePathFileName, string strRequestURL, string strPostFields = "", string strHeaderData = "", bool bVerbose = false, int nDelayTime = 60000, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);

//////////////////////////////////////////////////////////////////////
//函数功能:GET方法传入URL获取JSON字符串
//函数参数:
//		strJsonData		返回的JSON数据字符串
//		strRequestURL	下载地址URL
//		strHeaderData	要发送的头部数据字符串数组(\r\n为分隔符)
//		strPostFields	发送的POST域数据
//		bVerbose		是否为详细日志信息
//		nDelayTime		超时设置，默认为60000毫秒
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
int curl_http_get_data(string & strJsonData, string strRequestURL, string strHeaderData = "", string strPostFields = "", bool bVerbose = false, int nDelayTime = 60000, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);

//////////////////////////////////////////////////////////////////////
//函数功能:POST方法传入URL及参数获取JSON字符串
//函数参数:
//		strJsonData		返回的JSON数据字符串
//		strRequestURL	下载地址URL
//		strHeaderData	要发送的头部数据字符串数组(\r\n为分隔符)
//		strPostFields	发送的POST域数据
//		nDelayTime		超时设置，默认为60000毫秒
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
int curl_http_post_data(string & strJsonData, string strRequestURL, string strHeaderData = "", string strPostFields = "", bool bVerbose = false, int nDelayTime = 60000, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);

CURL * curl_http_init();
void curl_http_exit(CURL *pCurl);
void curl_http_print_cookies(CURL *pCurl);
int curl_http_post_data(CURL *pCurl, string & strJsonData, string strRequestURL, string strHeaderData = "", string strPostFields = "", bool bVerbose = false, int nDelayTime = 60000, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);
int curl_http_get_data(CURL * pCurl, string & strJsonData, string strRequestURL, string strHeaderData = "", string strPostFields = "", bool bVerbose = false, int nDelayTime = 60000, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);

int curl_http_post_data(CURL * pCurl, char * pJsonData, unsigned int nJsonDataSize, const char * pRequestUrl, const char * pHeaderData = "", const char * pPostFields = "", bool bVerbose = false, int nDelayTime = 60000, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);
int curl_http_get_data(CURL * pCurl, char * pJsonData, unsigned int nJsonDataSize, const char * pRequestUrl, const char * pHeaderData = "", const char * pPostFields = "", bool bVerbose = false, int nDelayTime = 60000, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);

void init_curl_env(CURL ** p_p_curl);
void exit_curl_env(CURL ** p_p_curl);
int visit_sites(CURL *p_curl, string strUrl, string strHeaderDataList,
	string strHeaderFileName = ("head.dat"), string strBodyFileName = ("body.html"),
	string strCookIEFileName = ("cookie.dat"), string strCookIEJarFileName = ("cookie.dat"),
	string strEncodingUncompressMethodType = (""), CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);

int visit_posts_sites(CURL *p_curl, string strUrl, string strHeaderDataList, string strPostFields,
	string strHeaderFileName = ("head.dat"), string strBodyFileName = ("body.html"),
	string strCookIEFileName = ("cookie.dat"), string strCookIEJarFileName = ("cookie.dat"),
	string strEncodingUncompressMethodType = (""), CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);
int visit_sites(string strUrl, string strHeaderDataList,
	string strHeaderFileName = ("head.dat"), string strBodyFileName = ("body.html"),
	string strCookIEFileName = ("cookie.dat"), string strCookIEJarFileName = ("cookie.dat"),
	string strEncodingUncompressMethodType = (""), CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);

int visit_posts_sites(string strUrl, string strHeaderDataList, string strPostFields,
	string strHeaderFileName = ("head.dat"), string strBodyFileName = ("body.html"),
	string strCookIEFileName = ("cookie.dat"), string strCookIEJarFileName = ("cookie.dat"),
	string strEncodingUncompressMethodType = (""), CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler = 0);
