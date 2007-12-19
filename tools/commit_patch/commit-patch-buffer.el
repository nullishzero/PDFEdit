;; Copyright 2003-2006 Jim Radford <radford@bleackbean.org> 
;;                 and David Caldwell <david@porkrind.org>, All Rights Reserved.
;; This code can be distributed under the terms of the GNU Public License
;; Version: 2.1

(require 'vc)
(require 'log-edit)

(defun commit-patch-buffer (buffer directory)
  "Commit the patch found in BUFFER applying it from DIRECTORY."
  (interactive "bBuffer to commit: \nDDirectory: ")
  (let* ((patch-files (with-temp-buffer
                        (let ((lsdiff (current-buffer)))
                          (when (eq 0 (with-current-buffer buffer
                                        (call-process-region (point-min) (point-max) 
                                                             "lsdiff" nil lsdiff nil)))
                            (split-string (buffer-string)))))) 
         (f patch-files) visiting-buffers)
    (while (car f)
      (let ((buf (find-buffer-visiting (car f))))
        (when buf
          (with-current-buffer buf (vc-buffer-sync))
          (add-to-list 'visiting-buffers buf)))
      (setq f (cdr f)))
    (log-edit
     `(lambda () (interactive)
        (let ((patch (make-temp-file "commit-buffer" nil))
              (comment (buffer-string))
              (output-buffer (window-buffer
                              (display-buffer
                               (get-buffer-create "*commit-patch*")))))
          (unwind-protect 
              (progn
                (with-current-buffer ,buffer
                  (write-region (point-min) (point-max) patch))
                (with-current-buffer output-buffer
                  (erase-buffer)
                  (let* ((default-directory ,directory) 
                         (status (call-process "commit-patch" nil
                                               output-buffer 'display
                                               "-m" comment patch)))
                    (if (not (eq status 0))
                        (message "Commit patch failed with a status of '%S' (%S)." status patch)
                      (mapc (lambda (buf) (with-current-buffer buf
                                            (vc-resynch-buffer (buffer-file-name buf) 'revert 'noquery)
                                            ;; stupid vc-revert-buffer1 doesn't call revert-buffer
                                            ;; with preserve-modes which means the CVS version doesn't
                                            ;; get updated, so we do it by hand.
                                            (run-hooks 'find-file-hooks)))
                            ',visiting-buffers)
                      (message "Patched and commited %S file(s) and reverted %S." 
                               ,(length patch-files) ,(length visiting-buffers))))))
            (delete-file patch))))
     nil
     `(lambda () ',patch-files)
     "*commit*")))

(when (require 'diff-mode)
  (setq diff-default-read-only nil)
  (define-key diff-mode-map "\C-c\C-c" 'commit-patch-buffer))

(provide 'commit-patch-buffer)
