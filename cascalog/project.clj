(defproject cascalog "0.1.0-SNAPSHOT"
  :description "FIXME: write description"
  :url "http://example.com/FIXME"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :repositories {"conjars"
                 {:url "http://conjars.org/repo"}}
  :dependencies [[org.clojure/clojure "1.6.0"]
                 [com.taoensso/timbre "3.3.1"]
                 [cascalog/cascalog-core "2.1.1"]
                 ;[cascalog/cascalog-more-taps "2.1.1"]
                 [cascading/cascading-core "2.5.3"]
                 ]
  :profiles
  {:provided {:dependencies
              [[org.apache.hadoop/hadoop-mapreduce-client-core "2.4.0"]
               [org.apache.hadoop/hadoop-minicluster "2.4.0"]
               [org.apache.hadoop/hadoop-common "2.4.0"]
               [commons-httpclient "3.0"]]}}

  :jvm-opts ^:replace ["-XX:MaxPermSize=128M"
                       "-XX:+UseConcMarkSweepGC"
                       "-Xms2g" "-Xmx2g" "-server"
                       ]
  )
