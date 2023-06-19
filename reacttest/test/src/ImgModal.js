import React from 'react';
import { Button, Modal, ModalHeader, ModalBody, ModalFooter } from 'reactstrap';
import 'bootstrap/dist/css/bootstrap.css';
//import OpenModal from './OpenModalContext';

const ImgModal = (props) => {
    return (
        <Modal isOpen={props.value} fade={true} toggle={props.show}>
            <ModalHeader toggle={props.onClose}>View Image</ModalHeader>
            <ModalBody>
                {props.CID}-CID가 잘 전달됨
            </ModalBody>
            <ModalFooter>
                <Button color='primary' onClick={props.onClose}>확인</Button>
            </ModalFooter>
        </Modal> 
    );
};

export default ImgModal;