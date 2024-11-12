require 'octokit'
require 'json'

token = ENV["GITHUB_TOKEN"]

client = Octokit::Client.new(:auth_token => token)
client.auto_paginate = true

event = JSON.parse( File.read("#{ENV["GITHUB_EVENT_PATH"]}") )
milestone = {number: event["milestone"]["number"], title: event["milestone"]["title"]} 

now = Time.now
news = Array.new

client.list_issues("#{ENV["GITHUB_REPOSITORY"]}", :milestone => milestone[:number], :state => "closed").each do |issue|
  news.push "##{issue.number}: #{issue.title} (#{issue.milestone.title}) [#{issue.labels.reduce(" ") {|r, label| r + label.name + " "}}]"
end

infile = File.open("#{ENV["GITHUB_WORKSPACE"]}/NEWS")
outfile = File.open("#{ENV["GITHUB_WORKSPACE"]}/NEWS.new", "w")

outfile.write infile.gets

outfile.write "\n"
outfile.write "* Noteworthy changes in release #{milestone[:title]} (#{now.year}-#{now.month}-#{now.day})\n"
outfile.write "\n"
news.each {|n| outfile.write "#{n}\n"}

infile.each {|l| outfile.write l}

infile.close
outfile.close

puts "#{milestone[:title]}"
