const fs = require('fs')
const config = require('../lib/config')
const util = require('../lib/util')

const mergeWithDefault = (options) => {
  return Object.assign({}, config.defaultOptions, options)
}

const runPylint = (args, continueOnFail = false) => {
  let cmd_options = config.defaultOptions
  cmd_options.cwd = config.braveCoreDir
  cmd_options.continueOnFail = continueOnFail
  cmd_options = mergeWithDefault(cmd_options)
  util.run('pylint', args, cmd_options)
}

const getChangedFiles = (base_branch, check_folders) => {
  const merge_base = util.runGit(config.braveCoreDir, ['merge-base', 'origin/' + base_branch, 'HEAD'])

  // Have to use shell because git doesn't understand quoted pathspecs.
  const run_options = { cwd: config.braveCoreDir, shell: true }
  const prog = util.run('git', ['diff', '--name-only', '--diff-filter',
    'drt', merge_base, '--', check_folders.join('/*.py ') + '/*.py'], run_options)
  const paths = prog.stdout.toString().trim().split('\n')
  return paths.length === 1 && paths[0] === '' ? [] : paths
}

const pylint = (options = {}) => {
  const check_folders = ['build', 'components', 'installer', 'script', 'tools']
  const report_file = 'pylint-report.txt'

  let paths = check_folders
  let description = 'pylint findings'
  if (options.base) {
    paths = getChangedFiles(options.base, check_folders)
    description += ' in scripts changed relative to ' + options.base
  }
  description += ' in: ' + check_folders.join('/ ') + '/'

  if (!paths.length) {
    console.log('No ' + description)
    if (options.report) {
      fs.closeSync(fs.openSync(report_file, 'w'))
    }
    return
  }

  let args = ['-j0', '-rn', '--rcfile=.pylintrc']
  if (options.report) {
    args.push('-fparseable')
  }
  args = args.concat(paths)
  if (options.report) {
    args.push('>' + report_file)
  }

  console.log('Checking for ' + description)
  runPylint(args, options.report)
}

module.exports = pylint
