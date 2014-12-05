(ns play
  (:require [clojure.core.async :as async
                                :refer [chan go thread timeout close! >! <! >!! <!! alts! alts!!]]))

(let [c (chan)]
  (go (>! c "hello"))
  (let [v (<!! (go (<! c)))]
    (assert (= "hello" v)))
  (close! c))

;; timers:
;; total until exit (then.exit - now)
;; time until stats snapshot (then.stats - now)
;; time until failure (then.to - now)

;; thread that writes is trivial; write ever 50ms, 10x.  then increase delay by 50ms each time ad infinitum.
(defn get-delay [n]
  (if (<= n 10)
    50
    (+ 50 (* 50 (- n 10)))))

;; easy test -- spawn writer thread w/ get-delay from above, run 15x
;; read 15x.  done.
#_(let [c (chan)]
  (thread (doseq [n (range 15)]
            (>!! c [n (get-delay n)])
            (Thread/sleep (get-delay n)))
          (close! c))
  (dotimes [i 15]
    (let [[n d] (<!! c)]
      (println [n d]))))

;; update, spawn infinite writer, arbitrary close from reader
(let [c (chan)]
  (thread (loop [n 0, cont? true]
            (let [delay (get-delay n)
                  cont (>!! c [n delay])]
              (Thread/sleep (get-delay n))
              (recur (inc n) cont))))
  (loop []
    (let [[v ch] (alts!! [c (timeout 175)])
          stop   (not= ch c)]
      (if stop
        (close! c)
        (do
          (println v)
          (recur))))))

;;
