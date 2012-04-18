#!/usr/bin/env ruby
# by Helder Ribeiro 2009
#
# Plugin to monitor delayed_jobs' queue size
# Gives updates with number of jobs that haven't been started yet
# plus the ones that failed and are still rescheduled for another run
#
# Parameters supported:
#
#  config

require 'rubygems'
require 'mysql'
require 'yaml'

class Grapher

  def initialize(db_conf)
    @db_conf = db_conf
  end

  def config
    puts <<-END_CONFIG
graph_title Delayed_Jobs Queue Size
graph_args -l 0
graph_vlabel jobs to be run
jobs.label jobs
jobs.type GAUGE
    END_CONFIG
  end

  def get_data
    mysql = Mysql.new(@db_conf['host'] || 'localhost',
                      @db_conf['username'] || 'root', @db_conf['password'], 
                      @db_conf['database'], @db_conf['port'],
                      @db_conf['socket'])
    result = mysql.query("SELECT count(*) FROM delayed_jobs WHERE \
                       first_started_at IS NULL OR run_at > NOW()")
    value = result.fetch_hash.values.first
    puts "jobs.value #{value}"
  end

end

if __FILE__ == $0

  environment = ENV['RAILS_ENV'] || 'production'
  db_conf = YAML.load(File.read(ENV['DATABASE_YML']))[environment]
  grapher = Grapher.new(db_conf)

  case ARGV.first
  when 'config'
    grapher.config
  else
    grapher.get_data
  end

end
