(defun build()
  (interactive)
  (let ((old-pwd (eshell/pwd))
	(old-buf (current-buffer)))
    (cd "~/kameleon/esp32/build")
    (compile "ninja")
    (set-buffer "*compilation*")
    (goto-char (point-max))
    (set-buffer old-buf)
    (cd old-pwd)))

	
