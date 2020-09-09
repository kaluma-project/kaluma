(defun build()
  (interactive)
  (let ((old-buf (current-buffer))
	(new-buf (get-buffer-create "*compilation*")))
    (set-buffer new-buf)
    (cd "~/Work/kameleon/esp32")
    (compile "bash build_all.sh")
    (goto-char (point-max))
    (set-buffer old-buf)))


	
