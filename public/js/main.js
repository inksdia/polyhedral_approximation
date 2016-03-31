$( document ).ready(function() {
	$("#outputImage").hide();
	$("#done").prop('disabled', true);
//	$("#batman").show();
});

$("#args").submit(function( event ) {
	var args = $("#args").serializeArray();
	
//	var viewh = $("#outputImage").height()/2;
//	var vieww = $("#outputImage").width()/2;
	var viewh = 400;
	var vieww = 400;
	
	args.push({name: 'h', value: viewh.toString()});
	args.push({name: 'w', value: vieww.toString()});
	
	$("#done").prop('disabled', true);
	
	console.log(args);
	
	$.ajax({
		type        : 'POST',
		url         : '/api/solve',
		data        : args,
		dataType    : 'text',
		encode      : true,
		cache				:	false,
		success			: function (data) {
			$("#outputImage").show();
			$("#batman").hide();
			$("#response").text(data);
			$("#done").prop('disabled', false);
			$("#insvg").attr("src", "input.svg?t=" + new Date().getTime());
			$("#outsvg").attr("src","output.svg?t=" + new Date().getTime());
		},
		error				: function (data) {
			$("#outputImage").hide();
			$("#batman").show();
			$("#response").text("Some Error!");
			$("#done").prop('disabled', false);
		}
	});

	event.preventDefault();
});

$("#uploadFile").change(function() {
	var formData = new FormData();
	formData.append('file', this.files[0]);

	$("#files").append($("#fileUploadProgressTemplate").tmpl());
	$("#fileUploadError").addClass("hide");
	
	$("#done").prop('disabled', false);

	$.ajax({
		url: '/api/uploads/',
		type: 'POST',
		xhr: function() {
			var xhr = $.ajaxSettings.xhr();
			if (xhr.upload) {
				xhr.upload.addEventListener('progress', function(evt) {
					var percent = (evt.loaded / evt.total) * 100;
					$("#files").find(".progress-bar").width(percent + "%");
				}, false);
			}
			return xhr;
		},
		success: function(data) {
			console.log(data);
			$("#files").children().last().remove();
			$("#files").append($("#fileUploadItemTemplate").tmpl(data));
			$("#uploadFile").closest("form").trigger("reset");
		},
		error: function() {
			$("#fileUploadError").removeClass("hide").text("An error occured!");
			$("#files").children().last().remove();
			$("#uploadFile").closest("form").trigger("reset");
		},
		data: formData,
		cache: false,
		contentType: false,
		processData: false
	}, 'json');
});