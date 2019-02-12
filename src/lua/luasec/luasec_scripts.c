/* luasocket_scripts.c */

#include <lua/lua/lua.h>
#include <lua/lua/lauxlib.h>
#include "luasec_scripts.h"

/* https.lua */
static const char *lua_sec_https = 
"----------------------------------------------------------------------------\n"
"-- LuaSec 0.7\n"
"-- Copyright (C) 2009-2018 PUC-Rio\n"
"--\n"
"-- Author: Pablo Musa\n"
"-- Author: Tomas Guisasola\n"
"---------------------------------------------------------------------------\n"
"\n"
"local socket = require(\"socket\")\n"
"local ssl    = require(\"ssl\")\n"
"local ltn12  = require(\"ltn12\")\n"
"local http   = require(\"socket.http\")\n"
"local url    = require(\"socket.url\")\n"
"\n"
"local try    = socket.try\n"
"\n"
"--\n"
"-- Module\n"
"--\n"
"local _M = {\n"
"  _VERSION   = \"0.7\",\n"
"  _COPYRIGHT = \"LuaSec 0.7 - Copyright (C) 2009-2018 PUC-Rio\",\n"
"  PORT       = 443,\n"
"  TIMEOUT    = 60\n"
"}\n"
"\n"
"-- TLS configuration\n"
"local cfg = {\n"
"  protocol = \"any\",\n"
"  options  = {\"all\", \"no_sslv2\", \"no_sslv3\", \"no_tlsv1\"},\n"
"  verify   = \"none\",\n"
"}\n"
"\n"
"--------------------------------------------------------------------\n"
"-- Auxiliar Functions\n"
"--------------------------------------------------------------------\n"
"\n"
"-- Insert default HTTPS port.\n"
"local function default_https_port(u)\n"
"   return url.build(url.parse(u, {port = _M.PORT}))\n"
"end\n"
"\n"
"-- Convert an URL to a table according to Luasocket needs.\n"
"local function urlstring_totable(url, body, result_table)\n"
"   url = {\n"
"      url = default_https_port(url),\n"
"      method = body and \"POST\" or \"GET\",\n"
"      sink = ltn12.sink.table(result_table)\n"
"   }\n"
"   if body then\n"
"      url.source = ltn12.source.string(body)\n"
"      url.headers = {\n"
"         [\"content-length\"] = #body,\n"
"         [\"content-type\"] = \"application/x-www-form-urlencoded\",\n"
"      }\n"
"   end\n"
"   return url\n"
"end\n"
"\n"
"-- Forward calls to the real connection object.\n"
"local function reg(conn)\n"
"   local mt = getmetatable(conn.sock).__index\n"
"   for name, method in pairs(mt) do\n"
"      if type(method) == \"function\" then\n"
"         conn[name] = function (self, ...)\n"
"                         return method(self.sock, ...)\n"
"                      end\n"
"      end\n"
"   end\n"
"end\n"
"\n"
"-- Return a function which performs the SSL/TLS connection.\n"
"local function tcp(params)\n"
"   params = params or {}\n"
"   -- Default settings\n"
"   for k, v in pairs(cfg) do \n"
"      params[k] = params[k] or v\n"
"   end\n"
"   -- Force client mode\n"
"   params.mode = \"client\"\n"
"   -- 'create' function for LuaSocket\n"
"   return function ()\n"
"      local conn = {}\n"
"      conn.sock = try(socket.tcp())\n"
"      local st = getmetatable(conn.sock).__index.settimeout\n"
"      function conn:settimeout(...)\n"
"         return st(self.sock, _M.TIMEOUT)\n"
"      end\n"
"      -- Replace TCP's connection function\n"
"      function conn:connect(host, port)\n"
"         try(self.sock:connect(host, port))\n"
"         self.sock = try(ssl.wrap(self.sock, params))\n"
"         self.sock:sni(host)\n"
"         self.sock:settimeout(_M.TIMEOUT)\n"
"         try(self.sock:dohandshake())\n"
"         reg(self, getmetatable(self.sock))\n"
"         return 1\n"
"      end\n"
"      return conn\n"
"  end\n"
"end\n"
"\n"
"--------------------------------------------------------------------\n"
"-- Main Function\n"
"--------------------------------------------------------------------\n"
"\n"
"-- Make a HTTP request over secure connection.  This function receives\n"
"--  the same parameters of LuaSocket's HTTP module (except 'proxy' and\n"
"--  'redirect') plus LuaSec parameters.\n"
"--\n"
"-- @param url mandatory (string or table)\n"
"-- @param body optional (string)\n"
"-- @return (string if url == string or 1), code, headers, status\n"
"--\n"
"local function request(url, body)\n"
"  local result_table = {}\n"
"  local stringrequest = type(url) == \"string\"\n"
"  if stringrequest then\n"
"    url = urlstring_totable(url, body, result_table)\n"
"  else\n"
"    url.url = default_https_port(url.url)\n"
"  end\n"
"  if http.PROXY or url.proxy then\n"
"    return nil, \"proxy not supported\"\n"
"  elseif url.redirect then\n"
"    return nil, \"redirect not supported\"\n"
"  elseif url.create then\n"
"    return nil, \"create function not permitted\"\n"
"  end\n"
"  -- New 'create' function to establish a secure connection\n"
"  url.create = tcp(url)\n"
"  local res, code, headers, status = http.request(url)\n"
"  if res and stringrequest then\n"
"    return table.concat(result_table), code, headers, status\n"
"  end\n"
"  return res, code, headers, status\n"
"end\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Export module\n"
"--\n"
"\n"
"_M.request = request\n"
"\n"
"return _M\n";


/* options.lua */
static const char *lua_sec_options = 
"local function usage()\n"
"  print(\"Usage:\")\n"
"  print(\"* Generate options of your system:\")\n"
"  print(\"  lua options.lua -g /path/to/ssl.h [version] > options.h\")\n"
"  print(\"* Examples:\")\n"
"  print(\"  lua options.lua -g /usr/include/openssl/ssl.h > options.h\\n\")\n"
"  print(\"  lua options.lua -g /usr/include/openssl/ssl.h \\\"OpenSSL 1.0.1 14\\\" > options.h\\n\")\n"
"\n"
"  print(\"* List options of your system:\")\n"
"  print(\"  lua options.lua -l /path/to/ssl.h\\n\")\n"
"end\n"
"\n"
"--\n"
"local function print(str, ...)\n"
"  print(string.format(str, ...))\n"
"end\n"
"\n"
"local function generate(options, version)\n"
"  print([[\n"
"#ifndef LSEC_OPTIONS_H\n"
"#define LSEC_OPTIONS_H\n"
"\n"
"/*--------------------------------------------------------------------------\n"
" * LuaSec 0.7\n"
" *\n"
" * Copyright (C) 2006-2018 Bruno Silvestre\n"
" *\n"
" *--------------------------------------------------------------------------*/\n"
"\n"
"#include <openssl/ssl.h>\n"
"\n"
"/* If you need to generate these options again, see options.lua */\n"
"]])\n"
"  print([[\n"
"/* \n"
"  OpenSSL version: %s\n"
"*/\n"
"]], version)\n"
"  print([[\n"
"struct ssl_option_s {\n"
"  const char *name;\n"
"  unsigned long code;\n"
"};\n"
"typedef struct ssl_option_s ssl_option_t;\n"
"]])\n"
"\n"
"  print([[static ssl_option_t ssl_options[] = {]])\n"
"\n"
"  for k, option in ipairs(options) do\n"
"    local name = string.lower(string.sub(option, 8))\n"
"    print(string.format([[#if defined(%s)]], option))\n"
"    print(string.format([[  {\"%s\", %s},]], name, option))\n"
"    print([[#endif]])\n"
"  end\n"
"  print([[  {NULL, 0L}]])\n"
"  print([[\n"
"};\n"
"\n"
"#endif\n"
"]])\n"
"end\n"
"\n"
"local function loadoptions(file)\n"
"  local options = {}\n"
"  local f = assert(io.open(file, \"r\"))\n"
"  for line in f:lines() do\n"
"    local op = string.match(line, \"define%s+(SSL_OP_%S+)\")\n"
"    if op then\n"
"      table.insert(options, op)\n"
"    end\n"
"  end\n"
"  table.sort(options, function(a,b) return a<b end)\n"
"  return options\n"
"end\n"
"--\n"
"\n"
"local options\n"
"local flag, file, version = ...\n"
"\n"
"version = version or \"Unknown\"\n"
"\n"
"if not file then\n"
"  usage()\n"
"elseif flag == \"-g\" then\n"
"  options = loadoptions(file)\n"
"  generate(options, version)\n"
"elseif flag == \"-l\" then\n"
"  options = loadoptions(file)\n"
"  for k, option in ipairs(options) do\n"
"    print(option)\n"
"  end\n"
"else\n"
"  usage()\n"
"end\n";


/* ssl.lua */
static const char *lua_sec_ssl = 
"------------------------------------------------------------------------------\n"
"-- LuaSec 0.7\n"
"--\n"
"-- Copyright (C) 2006-2018 Bruno Silvestre\n"
"--\n"
"------------------------------------------------------------------------------\n"
"\n"
"local core    = require(\"ssl.core\")\n"
"local context = require(\"ssl.context\")\n"
"local x509    = require(\"ssl.x509\")\n"
"local config  = require(\"ssl.config\")\n"
"\n"
"local unpack  = table.unpack or unpack\n"
"\n"
"-- We must prevent the contexts to be collected before the connections,\n"
"-- otherwise the C registry will be cleared.\n"
"local registry = setmetatable({}, {__mode=\"k\"})\n"
"\n"
"--\n"
"--\n"
"--\n"
"local function optexec(func, param, ctx)\n"
"  if param then\n"
"    if type(param) == \"table\" then\n"
"      return func(ctx, unpack(param))\n"
"    else\n"
"      return func(ctx, param)\n"
"    end\n"
"  end\n"
"  return true\n"
"end\n"
"\n"
"--\n"
"-- Convert an array of strings to wire-format\n"
"--\n"
"local function array2wireformat(array)\n"
"   local str = \"\"\n"
"   for k, v in ipairs(array) do\n"
"      if type(v) ~= \"string\" then return nil end\n"
"      local len = #v\n"
"      if len == 0 then\n"
"        return nil, \"invalid ALPN name (empty string)\"\n"
"      elseif len > 255 then\n"
"        return nil, \"invalid ALPN name (length > 255)\"\n"
"      end\n"
"      str = str .. string.char(len) .. v\n"
"   end\n"
"   if str == \"\" then return nil, \"invalid ALPN list (empty)\" end\n"
"   return str\n"
"end\n"
"\n"
"--\n"
"-- Convert wire-string format to array\n"
"--\n"
"local function wireformat2array(str)\n"
"   local i = 1\n"
"   local array = {}\n"
"   while i < #str do\n"
"      local len = str:byte(i)\n"
"      array[#array + 1] = str:sub(i + 1, i + len)\n"
"      i = i + len + 1\n"
"   end\n"
"   return array\n"
"end\n"
"\n"
"--\n"
"--\n"
"--\n"
"local function newcontext(cfg)\n"
"   local succ, msg, ctx\n"
"   -- Create the context\n"
"   ctx, msg = context.create(cfg.protocol)\n"
"   if not ctx then return nil, msg end\n"
"   -- Mode\n"
"   succ, msg = context.setmode(ctx, cfg.mode)\n"
"   if not succ then return nil, msg end\n"
"   -- Load the key\n"
"   if cfg.key then\n"
"      if cfg.password and\n"
"         type(cfg.password) ~= \"function\" and\n"
"         type(cfg.password) ~= \"string\"\n"
"      then\n"
"         return nil, \"invalid password type\"\n"
"      end\n"
"      succ, msg = context.loadkey(ctx, cfg.key, cfg.password)\n"
"      if not succ then return nil, msg end\n"
"   end\n"
"   -- Load the certificate\n"
"   if cfg.certificate then\n"
"     succ, msg = context.loadcert(ctx, cfg.certificate)\n"
"     if not succ then return nil, msg end\n"
"     if cfg.key and context.checkkey then\n"
"       succ = context.checkkey(ctx)\n"
"       if not succ then return nil, \"private key does not match public key\" end\n"
"     end\n"
"   end\n"
"   -- Load the CA certificates\n"
"   if cfg.cafile or cfg.capath then\n"
"      succ, msg = context.locations(ctx, cfg.cafile, cfg.capath)\n"
"      if not succ then return nil, msg end\n"
"   end\n"
"   -- Set SSL ciphers\n"
"   if cfg.ciphers then\n"
"      succ, msg = context.setcipher(ctx, cfg.ciphers)\n"
"      if not succ then return nil, msg end\n"
"   end\n"
"   -- Set the verification options\n"
"   succ, msg = optexec(context.setverify, cfg.verify, ctx)\n"
"   if not succ then return nil, msg end\n"
"   -- Set SSL options\n"
"   succ, msg = optexec(context.setoptions, cfg.options, ctx)\n"
"   if not succ then return nil, msg end\n"
"   -- Set the depth for certificate verification\n"
"   if cfg.depth then\n"
"      succ, msg = context.setdepth(ctx, cfg.depth)\n"
"      if not succ then return nil, msg end\n"
"   end\n"
"\n"
"   -- NOTE: Setting DH parameters and elliptic curves needs to come after\n"
"   -- setoptions(), in case the user has specified the single_{dh,ecdh}_use\n"
"   -- options.\n"
"\n"
"   -- Set DH parameters\n"
"   if cfg.dhparam then\n"
"      if type(cfg.dhparam) ~= \"function\" then\n"
"         return nil, \"invalid DH parameter type\"\n"
"      end\n"
"      context.setdhparam(ctx, cfg.dhparam)\n"
"   end\n"
"   \n"
"   -- Set elliptic curves\n"
"   if (not config.algorithms.ec) and (cfg.curve or cfg.curveslist) then\n"
"     return false, \"elliptic curves not supported\"\n"
"   end\n"
"   if config.capabilities.curves_list and cfg.curveslist then\n"
"     succ, msg = context.setcurveslist(ctx, cfg.curveslist)\n"
"     if not succ then return nil, msg end\n"
"   elseif cfg.curve then\n"
"     succ, msg = context.setcurve(ctx, cfg.curve)\n"
"     if not succ then return nil, msg end\n"
"   end\n"
"\n"
"   -- Set extra verification options\n"
"   if cfg.verifyext and ctx.setverifyext then\n"
"      succ, msg = optexec(ctx.setverifyext, cfg.verifyext, ctx)\n"
"      if not succ then return nil, msg end\n"
"   end\n"
"\n"
"   -- ALPN\n"
"   if cfg.mode == \"server\" and cfg.alpn then\n"
"      if type(cfg.alpn) == \"function\" then\n"
"         local alpncb = cfg.alpn\n"
"         -- This callback function has to return one value only\n"
"         succ, msg = context.setalpncb(ctx, function(str)\n"
"            local protocols = alpncb(wireformat2array(str))\n"
"            if type(protocols) == \"string\" then\n"
"               protocols = { protocols }\n"
"            elseif type(protocols) ~= \"table\" then\n"
"               return nil\n"
"            end\n"
"            return (array2wireformat(protocols))    -- use \"()\" to drop error message\n"
"         end)\n"
"         if not succ then return nil, msg end\n"
"      elseif type(cfg.alpn) == \"table\" then\n"
"         local protocols = cfg.alpn\n"
"         -- check if array is valid before use it\n"
"         succ, msg = array2wireformat(protocols)\n"
"         if not succ then return nil, msg end\n"
"         -- This callback function has to return one value only\n"
"         succ, msg = context.setalpncb(ctx, function()\n"
"            return (array2wireformat(protocols))    -- use \"()\" to drop error message\n"
"         end)\n"
"         if not succ then return nil, msg end\n"
"      else\n"
"         return nil, \"invalid ALPN parameter\"\n"
"      end\n"
"   elseif cfg.mode == \"client\" and cfg.alpn then\n"
"      local alpn\n"
"      if type(cfg.alpn) == \"string\" then\n"
"         alpn, msg = array2wireformat({ cfg.alpn })\n"
"      elseif type(cfg.alpn) == \"table\" then\n"
"         alpn, msg = array2wireformat(cfg.alpn)\n"
"      else\n"
"         return nil, \"invalid ALPN parameter\"\n"
"      end\n"
"      if not alpn then return nil, msg end\n"
"      succ, msg = context.setalpn(ctx, alpn)\n"
"      if not succ then return nil, msg end\n"
"   end\n"
"\n"
"   return ctx\n"
"end\n"
"\n"
"--\n"
"--\n"
"--\n"
"local function wrap(sock, cfg)\n"
"   local ctx, msg\n"
"   if type(cfg) == \"table\" then\n"
"      ctx, msg = newcontext(cfg)\n"
"      if not ctx then return nil, msg end\n"
"   else\n"
"      ctx = cfg\n"
"   end\n"
"   local s, msg = core.create(ctx)\n"
"   if s then\n"
"      core.setfd(s, sock:getfd())\n"
"      sock:setfd(core.SOCKET_INVALID)\n"
"      registry[s] = ctx\n"
"      return s\n"
"   end\n"
"   return nil, msg \n"
"end\n"
"\n"
"--\n"
"-- Extract connection information.\n"
"--\n"
"local function info(ssl, field)\n"
"  local str, comp, err, protocol\n"
"  comp, err = core.compression(ssl)\n"
"  if err then\n"
"    return comp, err\n"
"  end\n"
"  -- Avoid parser\n"
"  if field == \"compression\" then\n"
"    return comp\n"
"  end\n"
"  local info = {compression = comp}\n"
"  str, info.bits, info.algbits, protocol = core.info(ssl)\n"
"  if str then\n"
"    info.cipher, info.protocol, info.key,\n"
"    info.authentication, info.encryption, info.mac =\n"
"        string.match(str, \n"
"          \"^(%S+)%s+(%S+)%s+Kx=(%S+)%s+Au=(%S+)%s+Enc=(%S+)%s+Mac=(%S+)\")\n"
"    info.export = (string.match(str, \"%sexport%s*$\") ~= nil)\n"
"  end\n"
"  if protocol then\n"
"    info.protocol = protocol\n"
"  end\n"
"  if field then\n"
"    return info[field]\n"
"  end\n"
"  -- Empty?\n"
"  return ( (next(info)) and info )\n"
"end\n"
"\n"
"--\n"
"-- Set method for SSL connections.\n"
"--\n"
"core.setmethod(\"info\", info)\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Export module\n"
"--\n"
"\n"
"local _M = {\n"
"  _VERSION        = \"0.7\",\n"
"  _COPYRIGHT      = core.copyright(),\n"
"  loadcertificate = x509.load,\n"
"  newcontext      = newcontext,\n"
"  wrap            = wrap,\n"
"}\n"
"\n"
"return _M\n";


int luaopen_luasec_https(lua_State *L) {
    return luaL_loadstring(L, lua_sec_https);
}


int luaopen_luasec_options(lua_State *L) {
    return luaL_loadstring(L, lua_sec_options);
}


int luaopen_luasec_ssl(lua_State *L) {
    return luaL_loadstring(L, lua_sec_ssl);
}


static luaL_Reg luaopen_luasec_tbl[] = {
    {"ssl",         luaopen_luasec_ssl},
    {"ssl.options", luaopen_luasec_options},
    {"ssl.https",   luaopen_luasec_https},
    {NULL, NULL}
};

void luaopen_luasec_scripts(lua_State* L)
{
    luaL_Reg* lib = luaopen_luasec_tbl;
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    for (; lib->func; lib++)
    {
        lib->func(L);
        lua_setfield(L, -2, lib->name);
    }
    lua_pop(L, 2);
}
