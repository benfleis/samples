(ns play
  (:require [clojure.edn :as edn]
            [clojure.java.io :as io]
            [clojure.pprint :as pprint]
            [clojure.string :as string]

            ;[cascalog.api :as c]
            [cascalog.api :refer :all]
            [cascalog.logic.fn :as cfn]
            [cascalog.logic.ops :as cops]
            [cascalog.playground :refer :all]
            )
  )

; -----------------------------------------------------------------------------

(comment
(def lines (<- [?line] (sentence :> ?line)))
(def lengths (<- [?length] (lines :> ?line) (count ?line :> ?length)))

(defmapcatfn tokenize [line] (string/split line #"[\[\]\\\(\),.)\s]+"))
(def words (<- [?word] (lines :> ?line) (tokenize :< ?line :> ?word)))


(def counts (<- [?count] (words :> ?word) (cops/count :> ?count)))
(def word-counts (<- [?word ?count] (words :> ?word) (cops/count :> ?count)))

;(defn word-block-getter [n] (mapfn (comp (make-timestamp-block-getter n) count)))

;(def word-block-getter (comp (make-timestamp-block-getter n) count))

(defmapfn get-block [w] (* (long (/ (count w) 3)) 3))

;(def word-blocks-3 (<- [?block ?word] (words :> ?word) (get-block ?word :> ?block)))

(def plus0 (fnil + 0 0))
(defn combine-frequencies [x y]
  (merge-with plus0 x y))

(defparallelagg histo-pagg
  :init-var #'frequencies
  :combine-var #'combine-frequencies)

(defaggregatefn histo-agg
  ([] [{}])
  ([m word] [(combine-frequencies (first m) (frequencies word))])
  ([m] [m]))


(def histo-pagg-v2
  nil)

(def word-histo-by-blocks-3 (<- [?block ?histo]
                                (words :> ?word)
                                (get-block ?word :> ?block)
                                (histo-pagg ?word :> ?histo)))

(?- (stdout) word-histo-by-blocks-3)

 )
; -----------------------------------------------------------------------------

(defn print* [& xs]
  (println  "====")
  (pprint/pprint xs)
  (println  "----")
  true)

(def models [{:key "foo" :value 1 :weight 12}
             {:key "bar" :value 2 :weight 8}
             {:key "foo" :value 3 :weight 9}])
(defmapcatfn key->models [m-by-k k]
  (get m-by-k k))

(def encode-model pr-str)
(def decode-model read-string)

(def data [{:key "foo" :input 1} {:key "bar" :input 3}])
(defn data->key [d] (:key d))
(defn data->input [d] (:input d))

(defn q [ms m+d->result]
  (let [models-by-key (into {} (for [[k v] (group-by :key ms)] [k (map encode-model v)]))
        _ (pprint/pprint models-by-key)
        ]
    (<- [?model ?input ?result]
        (data :> ?datum)
        (get ?datum :key   :> ?key)
        (get ?datum :input :> ?input)
        (key->models models-by-key ?key :> ?model)
        (m+d->result ?model ?input :> ?result)
        (:distinct true)
        )))

(defn weighted [model-rep input]
  (* input (:weight (decode-model model-rep))))

#_(??- (q models weighted))

; -----------------------------------------------------------------------------
(comment

(def branch-data [0 1 2 3 4 5])
(defn transform-even [i] (<- [?res] (i :> ?n) (even? ?n) (+ ?n 2 :> ?res)))
(defn transform-odd  [i] (<- [?res] (i :> ?n) (odd? ?n) (* ?n 3 :> ?res)))
(?- (stdout)
    (transform-even branch-data)
    (stdout)
    (transform-odd branch-data))

(?<- (stdout)
     [?res]
     (branch-data :> ?n)
     ((fn [n] (if (even? n) n nil)) ?n :> ?even-n)
     (+ ?even-n 2 :> ?res)
     ((mapfn [n] (if (even? n) nil n)) ?n :> ?odd-n)
     (* ?odd-n 3 :> ?res))

)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; The goal here is to stitch together 2 disparate data sources (processed at
;; different times); the majority of the data is independent, but a few items
;; should be merged.  At the end, sink-templates should divide the data back
;; up.
;;

(def stitch-data
  [["a" "foo*" {:a 1}]
   ["b" "whah" {:b 1}]
   ["a" "foo*" {:p 3}]])

(defparallelagg stitch
  :init-var #'identity
  :combine-var #'merge)

(?<- (stdout)
     [?id ?match ?stitched]
     (stitch-data :> ?id ?match ?data)
     (stitch ?data :> ?stitched))

;; (spec: input dates, output dates, overwrite?)
;; multi-tap input
;; template-sink
