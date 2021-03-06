extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
}

#ifdef CR
#undef CR
#include <DEApplication.h>
#endif


DEApplication DE;

extern "C"
ngx_int_t init_cxxmvc_module(ngx_cycle_t *cycle)
{
	DE.Start();
	return 0;
}


extern "C"
void uninit_cxxmvc_module(ngx_cycle_t *cycle)
{
	DE.End();
}

/*
 *
 */
typedef struct {
	ngx_str_t path;
} ngx_http_cxxmvc_loc_conf_t;

/*
 *
 */
static void *ngx_http_cxxmvc_create_loc_conf(ngx_conf_t *cf);

/*
 *
 */
static char *ngx_http_cxxmvc_merge_loc_conf(ngx_conf_t *cf,
					    void *parent,
					    void *child);
/*
 *
 */
static char *ngx_http_cxxmvc_read_conf(ngx_conf_t *cf,
				       ngx_command_t *cmd,
				       void *conf);
/*
 *
 */
static ngx_int_t ngx_http_cxxmvc_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_cxxmvc_commands[] = {
	{
		ngx_string("set_app"),
		NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
		ngx_http_cxxmvc_read_conf,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_cxxmvc_loc_conf_t, path),
		NULL
	},
	ngx_null_command
};


static ngx_http_module_t ngx_http_cxxmvc_module_ctx = {
		NULL,
		NULL,

		NULL,
		NULL,

		NULL,
		NULL,

		ngx_http_cxxmvc_create_loc_conf,
		ngx_http_cxxmvc_merge_loc_conf
};

ngx_module_t ngx_http_dragon_module = {
		 NGX_MODULE_V1,
		 &ngx_http_cxxmvc_module_ctx,
		 ngx_http_cxxmvc_commands,
		 NGX_HTTP_MODULE,
		 NULL,
		 init_cxxmvc_module,
		 NULL,
		 NULL,
		 NULL,
		 NULL,
		 uninit_cxxmvc_module,
		 NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_http_add_header(ngx_http_request_t *r, ngx_str_t *value, ngx_str_t *key)
{
    ngx_table_elt_t  *cc;

    cc = (ngx_table_elt_t *)ngx_list_push(&r->headers_out.headers);
    if (cc == NULL) {
        return NGX_ERROR;
    }

    cc->hash = 1;
    cc->key.data = key->data;
    cc->key.len  = key->len;
    cc->value = *value;

    return NGX_OK;
}

static ngx_int_t
ngx_http_add_cookie(ngx_http_request_t *r, ngx_str_t value)
{
    ngx_table_elt_t  *cc;

    cc = (ngx_table_elt_t *)ngx_list_push(&r->headers_out.headers);
    if (cc == NULL) {
        return NGX_ERROR;
    }

    cc->hash = 1;
    ngx_str_set(&cc->key, "Set-Cookie");
    cc->value = value;

    return NGX_OK;
}

static std::string ngx_http_get_cookie(ngx_http_request_t *r);
static std::string ngx_http_get_language(ngx_http_request_t *r);

static ngx_int_t ngx_http_cxxmvc_handler(ngx_http_request_t *r)
{
	using dragon::kHttp_Method_Get;
	using dragon::kResponseTypeRedirect;
	using dragon::HeaderList;
	using dragon::CookieList;
	using dragon::StringRef;
	using dragon::HttpRequest;
	using dragon::HttpResponse;

	ngx_buf_t *buf = NULL;
	ngx_chain_t out;

	unsigned uriLen = r->uri.len;
	if (r->args.len > 0) 
		uriLen = r->uri.len + r->args.len + 1;
	
	struct sockaddr_in *sin;
	ngx_addr_t          addr;
	addr.sockaddr = r->connection->sockaddr;
	addr.socklen  = r->connection->socklen;
	sin = (struct sockaddr_in *)addr.sockaddr;
	char *ip = inet_ntoa(sin->sin_addr);

	StringRef IP(ip, strlen(ip));
	StringRef routingUrl((const char *)r->uri.data, uriLen);
	StringRef userAgent;
	if (r->headers_in.user_agent)
		userAgent = StringRef((const char *)r->headers_in.user_agent->value.data, r->headers_in.user_agent->value.len);
	
	std::string userCookie = ngx_http_get_cookie(r);
	std::string lang       = ngx_http_get_language(r);

	HttpRequest  req;
	HttpResponse res;

	req.SetMethod(kHttp_Method_Get);
	req.SetUrl(routingUrl);
	req.SetIp(IP);
	//req.SetHost(StringRef(host, strlen(host)));
	req.SetUserAgent(userAgent);
	req.SetUserCookie(StringRef(userCookie.c_str(), userCookie.length()));
	req.SetAcceptLanguage(StringRef(lang.c_str(), lang.length()));
        req.ParseCookie();

	DE.ResponseRequest(req, res);

	ngx_str_t k = ngx_string("X-Powered-By");
	ngx_str_t v = ngx_string("cxxmvc/0.1");;
	ngx_http_add_header(r, &v, &k);

	if (res.GetResponseType() == kResponseTypeRedirect)
	{
		ngx_str_t k = ngx_string("location");
		ngx_str_t v = {res.GetContent().length(), (u_char *)res.GetContent().c_str()};
		ngx_http_add_header(r, &v, &k);
	}

	HeaderList headers = res.GetHeaders();
	if (headers.size() > 0) 
	{
		HeaderList::iterator iter;
		HeaderList::iterator end = headers.end();
		for (iter = headers.begin(); iter != end; ++iter)
		{		
			ngx_str_t k;
			ngx_str_t v; 

			k.data = (u_char *)ngx_pcalloc(r->pool, iter->first.length()+1);
			k.len  = iter->first.length();
			strcpy((char *)k.data, (const char *)iter->first.c_str());

			v.data = (u_char *)ngx_pcalloc(r->pool, iter->second.length()+1);
			v.len  = iter->second.length();
			strcpy((char *)v.data, (const char *)iter->second.c_str());

		//	std::cout << "key : "<< k.data << std::endl;
		//	std::cout << "value : " << v.data << std::endl;

			ngx_http_add_header(r, &v, &k);
		}
	}

	CookieList cookies = res.GetCookies();
	if (cookies.size() > 0)
	{
		CookieList::iterator iter;
		CookieList::iterator end = cookies.end();
		for (iter = cookies.begin(); iter != end; ++iter)
		{
			ngx_str_t v; 

			v.data = (u_char *)ngx_pcalloc(r->pool, iter->length()+1);
			v.len  = iter->length();
			strcpy((char *)v.data, (const char *)iter->c_str());

			ngx_http_add_cookie(r, v);
		}
	}

	r->headers_out.status             = res.GetStatusCode();
    	r->headers_out.content_type.len   = res.GetContentType().length();
	r->headers_out.content_type.data  = (u_char *)res.GetContentType().c_str();

	if (res.GetDetaRef().length() > 0)
		r->headers_out.content_length_n = res.GetDetaRef().length();
        else 
       		r->headers_out.content_length_n = res.GetContent().length();


        ngx_http_send_header(r);

        buf = (ngx_buf_t *)ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

        if (buf == NULL) {
                ngx_log_error(NGX_LOG_ERR,
                              r->connection->log,
                              0,
                              "Failed to allocate response buffer.");
                return NGX_ERROR;
        }

	if (res.GetDetaRef().length() > 0) {
		buf->pos = (u_char *)res.GetDetaRef().data();
       		buf->last = buf->pos + res.GetDetaRef().length();
	} else {
       		buf->pos = (u_char *)res.GetContent().c_str();
        	buf->last = buf->pos + res.GetContent().length();
	}
        buf->memory   = 1; /* content is in read-only memory */
        buf->last_buf = 1; /* there will be no more buffers in the request */

	out.buf    = buf;
	out.next   = NULL;

	return ngx_http_output_filter(r, &out);
}

static char *ngx_http_cxxmvc_read_conf(ngx_conf_t *cf,
				       ngx_command_t *cmd,
				       void *conf)
{
	ngx_http_core_loc_conf_t *core_conf;

	core_conf = (ngx_http_core_loc_conf_t *)ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	core_conf->handler = ngx_http_cxxmvc_handler;
	ngx_conf_set_str_slot(cf, cmd, conf);

	return NGX_CONF_OK;
}

static void *ngx_http_cxxmvc_create_loc_conf(ngx_conf_t *cf)
{
	ngx_http_cxxmvc_loc_conf_t *conf;

	conf = (ngx_http_cxxmvc_loc_conf_t *)ngx_pcalloc(cf->pool, sizeof(ngx_http_cxxmvc_loc_conf_t));
	if (conf == NULL) {
		return NGX_CONF_ERROR;
	}

	conf->path.len 	= (size_t)NGX_CONF_UNSET_UINT;
	conf->path.data	= (u_char *)NGX_CONF_UNSET_PTR;

	return conf;
}

static char *ngx_http_cxxmvc_merge_loc_conf(ngx_conf_t *cf,
					      void *parent,
					      void *child)
{
	ngx_http_cxxmvc_loc_conf_t *prev  = (ngx_http_cxxmvc_loc_conf_t *)parent;
	ngx_http_cxxmvc_loc_conf_t *conf  = (ngx_http_cxxmvc_loc_conf_t *)child;

	ngx_conf_merge_str_value(conf->path, prev->path, "/usr/lib/");
	return NGX_CONF_OK;
}

static std::string ngx_http_get_cookie(ngx_http_request_t *r)
{
	ngx_table_elt_t **elt = NULL; 
	ngx_array_t cookies = r->headers_in.cookies;

	if (cookies.nelts > 0)
	{
		elt = (ngx_table_elt_t **)cookies.elts;
		return std::string((const char *)elt[0]->value.data, elt[0]->value.len);
	}

	return std::string();
}

static std::string ngx_http_get_language(ngx_http_request_t *r)
{
	if (r->headers_in.accept_language)
		return std::string((const char *) r->headers_in.accept_language->value.data, r->headers_in.accept_language->value.len);
	else
		return std::string();
}
