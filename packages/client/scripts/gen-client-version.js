#!/usr/bin/env node
const fs = require('fs');
const path = require('path');
const pkg = require('../package.json');
const content = `export const OMEGA_EDIT_CLIENT_VERSION = '${pkg.version}'\n`;
fs.writeFileSync(path.join(__dirname, '..', 'src', 'client_version.ts'), content);
