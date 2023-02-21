/*
 * Copyright (c) 2021 Concurrent Technologies Corporation.
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// @ts-nocheck <-- This is needed as this file is basically a JavaScript script
//                 but with some TypeScript niceness baked in
const AdmZip = require('adm-zip')
const fs = require('fs')
const path = require('path')
const glob = require('glob')
const execSync = require('child_process').execSync
const pkg_dir = 'dist/package'
const pkg_version = JSON.parse(fs.readFileSync('./package.json').toString())[
  'version'
]

function addExecutableBitToFile(filePath) {
  const stats = fs.statSync(filePath)
  if (stats.isFile()) {
    fs.chmodSync(filePath, stats.mode | 0o111)
  }
}

function addExecutableBitToFiles(directoryPath) {
  // Add the executable bit to each file in the directory
  fs.readdirSync(directoryPath).map((file) => {
    addExecutableBitToFile(path.join(directoryPath, file))
  })
}

function copyGlob(pattern, destDir = pkg_dir, dir = '.') {
  glob(pattern, { cwd: dir }, (error, files) => {
    for (let i = 0; i < files.length; i++) {
      const src = path.join(dir, files[i])
      const dst = path.join(destDir, path.parse(files[i]).base)
      const dstDir = path.dirname(dst)

      fs.mkdirSync(dstDir, { recursive: true })

      if (fs.statSync(src).isFile()) {
        fs.copyFileSync(src, dst)
      }
    }
  })
}

// Setup package directory
function setup() {
  if (fs.existsSync(pkg_dir)) {
    fs.rmSync(pkg_dir, { recursive: true, force: true })
  }

  fs.mkdirSync(pkg_dir, { recursive: true })

  copyGlob('src/*.d.ts')
  copyGlob('src/*.js')
  copyGlob('out/*')

  // Unpack the omega-edit-grpc-server zip file to the package directory
  const omegaEditServer = `omega-edit-grpc-server-${pkg_version}.zip`
  new AdmZip(omegaEditServer).extractAllTo(pkg_dir, true)
  addExecutableBitToFiles(
    `${pkg_dir}/omega-edit-grpc-server-${pkg_version}/bin`
  )

  fs.copyFileSync('yarn.lock', `${pkg_dir}/yarn.lock`)
  fs.copyFileSync('package.json', `${pkg_dir}/package.json`)
  fs.copyFileSync('.npmignore', `${pkg_dir}/.npmignore`)
  fs.copyFileSync('README.md', `${pkg_dir}/README.md`)
}

// Create package
function create() {
  execSync('yarn install', { cwd: pkg_dir })
  execSync(`yarn pack --cwd ${pkg_dir}`)
  const packageFile = `omega-edit-v${pkg_version}.tgz`
  fs.renameSync(path.join(pkg_dir, packageFile), packageFile)
}

module.exports = {
  setup: setup,
  create: create,
}
